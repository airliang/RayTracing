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
		//����Ϊ�β�ֱ��ȡlight->nSample����Ϊ��ĳЩ���nSampleҪRound
		const Point2f* uLightArray = sampler.Get2DArray(nSamples);
		const Point2f* uScatteringArray = sampler.Get2DArray(nSamples);
		if (!uLightArray || !uScatteringArray)
		{
			//����light�ϵ�incident vector���������
			Point2f uLight = sampler.Get2D();
			//����interaction point�ϵ�incident radiance vector���������
			Point2f uScattering = sampler.Get2D();
			L += EstimateDirect(it, uScattering, *light, uLight, scene, sampler, arena, handleMedia);
		}
		else
		{
			Spectrum Ld(0.f);
			for (int k = 0; k < nSamples; ++k)
				Ld += EstimateDirect(it, uScatteringArray[k], *light, uLightArray[k],
					scene, sampler, arena, handleMedia);
			//EstimateDirect�����light
			//E(Fn) = 1/N��f(Xi)/p(Xi)
			//���ʱ��EstimateDirect�൱����ʽ�е�f(x)
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
		//��lightDistrib�ķֲ�ȥȡ�ĸ�light��
		lightIndex = lightDistrib->SampleDiscrete(sampler.Get1D(), &lightPdf);
		if (lightPdf == 0)
			return L;
	}
	else
	{
		//û�зֲ�������£�����light��ȡ�õĸ�����ͬ
		lightIndex = std::min((int)(sampler.Get1D() * nLights), nLights - 1);
		lightPdf = Float(1) / nLights;
	}

	const std::shared_ptr<Light>& light = scene.lights[lightIndex];
	Point2f uLight = sampler.Get2D();
	Point2f uScattering = sampler.Get2D();
	//�����ؿ�����ƣ���ʵ�ǹ���EstimateDirect�Ļ���
	//pdf����ȡ��ĳ��light�ĸ���ֵ
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
	//���ȶԹ�Դ���в���
	Spectrum Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &visibility);
	if (lightPdf > 0 && !Li.IsBlack())
	{
		//light����ͨ�����ټ���bsdf����ͨ���Ļ�û��Ҫ����
		Spectrum f;
		if (it.IsSurfaceInteraction())
		{
			const Interaction& isect = (const Interaction&)it;
			f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * Vector3f::AbsDot(wi, isect.shading.n);
			scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);
		}
		else
		{
			//���������ʵ�radiance
		}
			

		if (!f.IsBlack())
		{
			if (handleMedia)
				Li *= visibility.Tr(scene, sampler);
			else if (!visibility.Unoccluded(scene))
				Li = Spectrum(0.f);


			// Add light's contribution to reflected radiance
			//Multiple Importance Sampling 
			//Lo(p,wo) = ��f(p,wo,i)Ld(p,wi)|cos��i|dwi
			//f�������bsdf->f������Ld��Sample_Li�ó��ĺ������ֱ���������f,g����
			//            f(Xi)g(Xi)wf(Xi)          f(Yi)g(Yi)wg(Yi)
			//MIS = 1/nf��----------------- + 1/ng��---------------
			//                  pf(Xi)                    pg(Yi)
			//
            //�����Ǽ���MIS�ұߵ�����
			if (!Li.IsBlack()) 
			{
				if (light.IsDeltaLight())
					Ld += f * Li / lightPdf;
				else 
				{
					//����nf = ng = n
					//���湫ʽ���԰�nԼ������ˣ����԰�1����
					//PowerHeuristic
					//������Կ���https://blog.csdn.net/air_liang1212/article/details/105253142
					Float weight =
						PowerHeuristic(1, lightPdf, 1, scatteringPdf);
					Ld += f * Li * weight / lightPdf;
				}
			}
		}
	}

	//����bsdf���������Light��DeltaLight��bsdf������
    //��Ϊdeltalightֻ�в�����Դ��������
    if (!light.IsDeltaLight())
    {
		//Lo(p,wo) = ��f(p,wo,i)Ld(p,wi)|cos��i|dwi
		//�����ȼ���f������bsdf��radiance�������
        Spectrum f; 
		bool sampledSpecular = false;
		if (it.IsSurfaceInteraction())
		{
			BxDFType sampledType;
			const Interaction &isect = (const Interaction &)it;
			f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf, bsdfFlags, &sampledType);
			sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
		}
        else
		{
		}

		//f�����û���ף���ǰ���Light��������Ϳ�����
		//û��Ҫ�ظ�����
		if (!f.IsBlack() && scatteringPdf > 0) 
		{
			Float weight = 1;
			//����MIS����ߵ���
			if (!sampledSpecular) 
			{
				lightPdf = light.Pdf_Li(it, wi);
				if (lightPdf == 0)
					return Ld;
				weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
			}

			//wiȷ�Ϻ�Ҫ������wi�ͳ�����arealight��û�ཻ
			//����ͳ�����ĳ��geometry�ཻ�����geometry�Դ���arealight
			//����Ҫ���ض�Ӧ��radiance
			Interaction lightIsect;
			Ray ray = it.SpawnRay(wi);
			Spectrum Tr(1.f);
			bool foundSurfaceInteraction = handleMedia ?
				  scene.IntersectTr(ray, sampler, &lightIsect, &Tr) :
				  scene.Intersect(ray, &lightIsect);

			//������Ⱦ���̵�Ld
			Spectrum Li(0.f);
			if (foundSurfaceInteraction)
			{
				//�÷����д�ʵ��
				//if (lightIsect.primitive->GetAreaLight() == &light)
				//	Li = lightIsect.Le(-wi);
			}
			else
			{
				//�Ҳ�������ʹ��InfiniteAreaLight��LiEscape
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

	//��Ⱦimage tiles
	//����image tile
	//��Ϊ�����ͼ��һ����film��(0, 0)��ʼ
	//����sampleBounds����film����ɫ����
	Bounds2i sampleBounds = camera->film->GetOutputSampleBounds();
	//���image�Ĵ�С
	Vector2i sampleExtent = sampleBounds.Diagonal();
	const int tileSize = 16;

	//nTiles�Ķ�ά����
	//����ͼ���С���ܱ�16����������x = 50, y = 50
	//(int)x/16 = 3����Ȼ��������
	//(int)(x + tileSize - 1)/16 = 4
	Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
		(sampleExtent.y + tileSize - 1) / tileSize);

	//tile�ǵڼ���tile
	ParallelFor2D([&](Point2i tile) {
		MemoryArena arena;
		int seed = tile.y * nTiles.x + tile.x;
		std::unique_ptr<Sampler> tileSampler = sampler->Clone(seed);

		//�������tile��sampleBound�ܵ�λ��
		int x0 = sampleBounds.pMin.x + tile.x * tileSize;
		int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
		int y0 = sampleBounds.pMin.y + tile.y * tileSize;
		int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
		Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

		std::unique_ptr<FilmTile> filmTile =
			camera->film->GetFilmTile(tileBounds);

		for (Point2i pixel : tileBounds)
		{
			//���ɸ�pixel��samples
			tileSampler->StartPixel(pixel);
			do 
			{
				//���ɵ�ǰpixel��cameraSample
				CameraSample cameraSample = tileSampler->GetCameraSample(pixel);
				RayDifferential ray;
				Float rayWeight = camera->GenerateRayDifferential(cameraSample, &ray);
				ray.ScaleDifferentials(1 / std::sqrt(tileSampler->samplesPerPixel));


				Spectrum L(0.f);
				//�����������·����radiance arriving at the film
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

Spectrum SamplerIntegrator::SpecularReflect(const RayDifferential& ray, const Interaction& isect, const Scene& scene,
	Sampler& sampler, MemoryArena& arena, int depth) const
{
    //�����wo
	Vector3f wo = isect.wo;
	//�����wi
	Vector3f wi;
	Float pdf;

	BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
	//get the perfectly specular vector wi
	Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);
	const Vector3f& ns = isect.shading.n;
	if (pdf != 0 && !f.IsBlack())
	{
		//����wi�����ray
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

Spectrum SamplerIntegrator::SpecularTransmit(const RayDifferential& ray, const Interaction& isect,
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
		//����wi�����ray
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
