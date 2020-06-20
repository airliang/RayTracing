#include "integrator.h"
#include "scene.h"
#include "light.h"
#include "sampling.h"
#include "bsdf.h"
#include "film.h"
#include "parallelism.h"

namespace AIR
{
Spectrum UniformSampleAllLights(const Interaction& it,
	const Scene& scene, MemoryArena& arena, Sampler& sampler,
	const std::vector<int>& nLightSamples, bool handleMedia)
{
	Spectrum L(0.f);
	for (size_t j = 0; j < scene.lights.size(); ++j) 
	{
		const std::shared_ptr<Light>& light = scene.lights[j];
		int nSamples = nLightSamples[j];
		//这里为何不直接取light->nSample，因为在某些情况nSample要Round
		const Point2f* uLightArray = sampler.Get2DArray(nSamples);
		const Point2f* uScatteringArray = sampler.Get2DArray(nSamples);
		if (!uLightArray || !uScatteringArray)
		{
			//采样light上的incident vector的随机样本
			Point2f uLight = sampler.Get2D();
			//采样interaction point上的incident radiance vector的随机样本
			Point2f uScattering = sampler.Get2D();
			L += EstimateDirect(it, uScattering, *light, uLight, scene, sampler, arena, handleMedia);
		}
		else
		{
			Spectrum Ld(0.f);
			for (int k = 0; k < nSamples; ++k)
				Ld += EstimateDirect(it, uScatteringArray[k], *light, uLightArray[k],
					scene, sampler, arena, handleMedia);
			//EstimateDirect算的是light
			//E(Fn) = 1/N∑f(Xi)/p(Xi)
			//这个时候EstimateDirect相当于上式中的f(x)
			L += Ld / nSamples;
		}
	}
	return L;
}

Spectrum UniformSampleOneLight(const Interaction& it, const Scene& scene,
	MemoryArena& arena, Sampler& sampler,
	bool handleMedia,
	const Distribution1D* lightDistrib)
{
	Spectrum L(0.f);
	//randomly choose a single light to sample
	int nLights = (int)scene.lights.size();
	if (nLights == 0)
		return L;

	int lightIndex = 0;
	Float lightPdf = 0;
	if (lightDistrib)
	{
		//按lightDistrib的分布去取哪个light。
		lightIndex = lightDistrib->SampleDiscrete(sampler.Get1D(), &lightPdf);
		if (lightPdf == 0)
			return L;
	}
	else
	{
		//没有分布的情况下，所有light的取得的概率相同
		lightIndex = std::min((int)(sampler.Get1D() * nLights), nLights - 1);
		lightPdf = Float(1) / nLights;
	}

	const std::shared_ptr<Light>& light = scene.lights[lightIndex];
	Point2f uLight = sampler.Get2D();
	Point2f uScattering = sampler.Get2D();
	//按蒙特卡洛估计，其实是估计EstimateDirect的积分
	//pdf就是取得某个light的概率值
	L = EstimateDirect(it, uScattering, *light, uLight,
		scene, sampler, arena, handleMedia) / lightPdf;

	return L;
}

Spectrum EstimateDirect(const Interaction& it,
	const Point2f& uScattering, const Light& light,
	const Point2f& uLight, const Scene& scene, Sampler& sampler,
	MemoryArena& arena, bool handleMedia, bool specular)
{
	BxDFType bsdfFlags = specular ? BSDF_ALL :
		BxDFType(BSDF_ALL & ~BSDF_SPECULAR);

	Spectrum Ld(0.f);

	Vector3f wi;
	Float lightPdf = 0, scatteringPdf = 0;
	VisibilityTester visibility;
	//首先对光源进行采样
	Spectrum Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &visibility);
	if (lightPdf > 0 && !Li.IsBlack())
	{
		//light采样通过后再计算bsdf，不通过的话没必要计算
		Spectrum f;
		if (it.IsSurfaceInteraction())
		{
			const SurfaceInteraction& isect = (const SurfaceInteraction&)it;
			f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * Vector3f::AbsDot(wi, isect.shading.n);
			scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);
		}
		else
		{
			//处理参与介质的radiance
			const MediumInteraction& mi = (const MediumInteraction&)it;
			Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
			f = Spectrum(p);
			scatteringPdf = p;
		}
			

		if (!f.IsBlack())
		{
			if (handleMedia)
				Li *= visibility.Tr(scene, sampler);
			else if (!visibility.Unoccluded(scene))
				Li = Spectrum(0.f);


			// Add light's contribution to reflected radiance
			//Multiple Importance Sampling 
			//Lo(p,wo) = ∫f(p,wo,i)Ld(p,wi)|cosθi|dwi
			//f是上面的bsdf->f函数，Ld是Sample_Li得出的函数，分别代表下面的f,g函数
			//            f(Xi)g(Xi)wf(Xi)          f(Yi)g(Yi)wg(Yi)
			//MIS = 1/nf∑----------------- + 1/ng∑---------------
			//                  pf(Xi)                    pg(Yi)
			//
            //下面是计算MIS右边的那项
			if (!Li.IsBlack()) 
			{
				if (light.IsDeltaLight())
					Ld += f * Li / lightPdf;
				else 
				{
					//由于nf = ng = n
					//上面公式可以把n约掉，因此，可以把1传进
					//PowerHeuristic
					//详情可以看：https://blog.csdn.net/air_liang1212/article/details/105253142
					Float weight =
						PowerHeuristic(1, lightPdf, 1, scatteringPdf);
					Ld += f * Li * weight / lightPdf;
				}
			}
		}
	}

	//处理bsdf采样，如果Light是DeltaLight，bsdf不处理，
    //因为deltalight只有采样光源才起作用
    if (!light.IsDeltaLight())
    {
		//Lo(p,wo) = ∫f(p,wo,i)Ld(p,wi)|cosθi|dwi
		//下面先计算f，代表bsdf的radiance反射比率
        Spectrum f; 
		bool sampledSpecular = false;
		if (it.IsSurfaceInteraction())
		{
			BxDFType sampledType;
			const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
			f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf, bsdfFlags, &sampledType);
			sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
		}
        else
		{
			//处理参与介质的radiance
			const MediumInteraction& mi = (const MediumInteraction&)it;
			Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
			f = Spectrum(p);
			scatteringPdf = p;
		}

		//f如果是没贡献，用前面的Light采样结果就可以了
		//没必要重复计算
		if (!f.IsBlack() && scatteringPdf > 0) 
		{
			Float weight = 1;
			//计算MIS的左边的项
			if (!sampledSpecular) 
			{
				lightPdf = light.Pdf_Li(it, wi);
				if (lightPdf == 0)
					return Ld;
				weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
			}

			//wi确认后，要检测这个wi和场景的arealight有没相交
			//假设和场景的某个geometry相交，这个geometry自带了arealight
			//就需要返回对应的radiance
			SurfaceInteraction lightIsect;
			Ray ray = it.SpawnRay(wi);
			Spectrum Tr(1.f);
			bool foundSurfaceInteraction = handleMedia ?
				  scene.IntersectTr(ray, sampler, &lightIsect, &Tr) :
				  scene.Intersect(ray, &lightIsect);

			//计算渲染方程的Ld
			Spectrum Li(0.f);
			if (foundSurfaceInteraction)
			{
				//该方法有待实现
				//if (lightIsect.primitive->GetAreaLight() == &light)
				//	Li = lightIsect.Le(-wi);
			}
			else
			{
				//找不到，那使用InfiniteAreaLight的LiEscape
				Li = light.LiEscape(ray);
			}
				

			if (!Li.IsBlack())
				Ld += f * Li * weight / scatteringPdf;
		}
    }

	return Ld;
}

void SamplerIntegrator::Render(const Scene& scene)
{
	Preprocess(scene, *sampler);

	//渲染image tiles
	//划分image tile
	//因为输出的图像不一定从film的(0, 0)开始
	//所以sampleBounds才是film的着色区域
	Bounds2i sampleBounds = camera->film->GetOutputSampleBounds();
	//获得image的大小
	Vector2i sampleExtent = sampleBounds.Diagonal();
	const int tileSize = 16;

	//nTiles的二维数组
	//假如图像大小不能被16整除，例如x = 50, y = 50
	//(int)x/16 = 3，显然数量不够
	//(int)(x + tileSize - 1)/16 = 4
	Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
		(sampleExtent.y + tileSize - 1) / tileSize);

	//tile是第几个tile
	ParallelFor2D([&](Point2i tile) {
		MemoryArena arena;
		int seed = tile.y * nTiles.x + tile.x;
		std::unique_ptr<Sampler> tileSampler = sampler->Clone(seed);

		//计算这个tile在sampleBound总的位置
		int x0 = sampleBounds.pMin.x + tile.x * tileSize;
		int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
		int y0 = sampleBounds.pMin.y + tile.y * tileSize;
		int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
		Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

		std::unique_ptr<FilmTile> filmTile =
			camera->film->GetFilmTile(tileBounds);

		for (Point2i pixel : tileBounds)
		{
			//生成该pixel的samples
			tileSampler->StartPixel(pixel);
			do 
			{
				//生成当前pixel的cameraSample
				CameraSample cameraSample = tileSampler->GetCameraSample(pixel);
				RayDifferential ray;
				Float rayWeight = camera->GenerateRayDifferential(cameraSample, &ray);
				ray.ScaleDifferentials(1 / std::sqrt(tileSampler->samplesPerPixel));


				Spectrum L(0.f);
				//下面计算整条路径的radiance arriving at the film
				if (rayWeight > 0) 
					L = Li(ray, scene, *tileSampler, arena);

				if (L.HasNaNs()) {
					//Error("Not-a-number radiance value returned "
					//	"for image sample.  Setting to black.");
					L = Spectrum(0.f);
				}
				else if (L.y() < -1e-5) {
					//Error("Negative luminance value, %f, returned "
					//	"for image sample.  Setting to black.", L.y());
					L = Spectrum(0.f);
				}
				else if (std::isinf(L.y())) {
					//Error("Infinite luminance value returned "
					//	"for image sample.  Setting to black.");
					L = Spectrum(0.f);
				}

				filmTile->AddSample(cameraSample.pFilm, L, rayWeight);
				arena.Reset();
			} while (tileSampler->StartNextSample());
		}

		camera->film->MergeFilmTile(std::move(filmTile));
	}, nTiles);

	camera->film->WriteImage();
}

Spectrum SamplerIntegrator::SpecularReflect(const RayDifferential& ray, const SurfaceInteraction& isect, const Scene& scene,
	Sampler& sampler, MemoryArena& arena, int depth) const
{
    //出射光wo
	Vector3f wo = isect.wo;
	//入射光wi
	Vector3f wi;
	Float pdf;

	BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
	//get the perfectly specular vector wi
	Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);
	const Vector3f& ns = isect.shading.n;
	if (pdf != 0 && !f.IsBlack())
	{
		//生成wi方向的ray
		RayDifferential rd = isect.SpawnRay(wi);
		if (ray.hasDifferentials)
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = isect.interactPoint + isect.dpdx;
			rd.ryOrigin = isect.interactPoint + isect.dpdy;
			// Compute differential reflected directions
			//chain rule for derivative
			//dndx = dn/du * du/dx + dn/dv * dv/dx
			Vector3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Vector3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;

			//dwo/dx = 
			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Vector3f::Dot(dwodx, ns) + Vector3f::Dot(wo, dndx);
			Float dDNdy = Vector3f::Dot(dwody, ns) + Vector3f::Dot(wo, dndy);
			rd.rxDirection =
				wi - dwodx + 2.f * Vector3f(Vector3f::Dot(wo, ns) * dndx + dDNdx * ns);
			rd.ryDirection =
				wi - dwody + 2.f * Vector3f(Vector3f::Dot(wo, ns) * dndy + dDNdy * ns);
		}

		return f * Li(rd, scene, sampler, arena, depth + 1) * Vector3f::AbsDot(wi, ns) /
			pdf;
	}

	return Spectrum(0.0f);
}

Spectrum SamplerIntegrator::SpecularTransmit(const RayDifferential& ray, const SurfaceInteraction& isect,
	const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const
{
	Vector3f wo = isect.wo;
	Vector3f wi;
	Float pdf;

	BxDFType type = BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR);
	//get the perfectly specular vector wi
	Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);
	Vector3f ns = isect.shading.n;
	if (pdf != 0 && !f.IsBlack())
	{
		//生成wi方向的ray
		RayDifferential rd = isect.SpawnRay(wi);
		if (ray.hasDifferentials)
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = isect.interactPoint + isect.dpdx;
			rd.ryOrigin = isect.interactPoint + isect.dpdy;
			// Compute differential reflected directions
			Vector3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Vector3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;
			
			// The BSDF stores the IOR of the interior of the object being
			// intersected.  Compute the relative IOR by first out by
			// assuming that the ray is entering the object.
			Float eta = 1 / isect.bsdf->eta;
			if (Vector3f::Dot(wo, ns) < 0) 
			{
				// If the ray isn't entering, then we need to invert the
				// relative IOR and negate the normal and its derivatives.
				eta = 1 / eta;
				ns = -ns;
				dndx = -dndx;
				dndy = -dndy;
			}

			/*
			  Notes on the derivation:
			  - pbrt computes the refracted ray as: \wi = -\eta \omega_o + [ \eta (\wo \cdot \N) - \cos \theta_t ] \N
				It flips the normal to lie in the same hemisphere as \wo, and then \eta is the relative IOR from
				\wo's medium to \wi's medium.
			  - If we denote the term in brackets by \mu, then we have: \wi = -\eta \omega_o + \mu \N
			  - Now let's take the partial derivative. (We'll use "d" for \partial in the following for brevity.)
				We get: -\eta d\omega_o / dx + \mu dN/dx + d\mu/dx N.
			  - We have the values of all of these except for d\mu/dx (using bits from the derivation of specularly
				reflected ray deifferentials).
			  - The first term of d\mu/dx is easy: \eta d(\wo \cdot N)/dx. We already have d(\wo \cdot N)/dx.
			  - The second term takes a little more work. We have:
				 \cos \theta_i = \sqrt{1 - \eta^2 (1 - (\wo \cdot N)^2)}.
				 Starting from (\wo \cdot N)^2 and reading outward, we have \cos^2 \theta_o, then \sin^2 \theta_o,
				 then \sin^2 \theta_i (via Snell's law), then \cos^2 \theta_i and then \cos \theta_i.
			  - Let's take the partial derivative of the sqrt expression. We get:
				1 / 2 * 1 / \cos \theta_i * d/dx (1 - \eta^2 (1 - (\wo \cdot N)^2)).
			  - That partial derivatve is equal to:
				d/dx \eta^2 (\wo \cdot N)^2 = 2 \eta^2 (\wo \cdot N) d/dx (\wo \cdot N).
			  - Plugging it in, we have d\mu/dx =
				\eta d(\wo \cdot N)/dx - (\eta^2 (\wo \cdot N) d/dx (\wo \cdot N))/(-\wi \cdot N).
			 */
			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Vector3f::Dot(dwodx, ns) + Vector3f::Dot(wo, dndx);
			Float dDNdy = Vector3f::Dot(dwody, ns) + Vector3f::Dot(wo, dndy);

			Float mu = eta * Vector3f::Dot(wo, ns) - Vector3f::AbsDot(wi, ns);
			Float dmudx =
				(eta - (eta * eta * Vector3f::Dot(wo, ns)) / Vector3f::AbsDot(wi, ns)) * dDNdx;
			Float dmudy =
				(eta - (eta * eta * Vector3f::Dot(wo, ns)) / Vector3f::AbsDot(wi, ns)) * dDNdy;

			rd.rxDirection =
				wi - eta * dwodx + Vector3f(mu * dndx + dmudx * ns);
			rd.ryDirection =
				wi - eta * dwody + Vector3f(mu * dndy + dmudy * ns);
		}

		return f * Li(rd, scene, sampler, arena, depth + 1) * Vector3f::AbsDot(wi, ns) /
			pdf;
	}

	return Spectrum(0.0f);
}

}
