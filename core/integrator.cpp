#include "integrator.h"
#include "scene.h"
#include "light.h"
#include "sampling.h"
#include "bsdf.h"

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
			const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
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
			SurfaceInteraction lightIsect;
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
				

			if (Li.IsBlack()!)
				Ld += f * Li * weight / scatteringPdf;
		}
    }

	return Ld;
}

}
