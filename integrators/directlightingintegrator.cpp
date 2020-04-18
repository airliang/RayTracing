#include "directlightingintegrator.h"
#include "scene.h"
#include "light.h"

namespace AIR
{
	void DirectLightingIntegrator::Preprocess(const Scene& scene,
		Sampler& sampler)
	{
		if (strategy == LightStrategy::UniformSampleAll)
		{
			for (const auto &light : scene.lights)
			{
				nLightSamples.push_back(sampler.RoundCount(light->nSamples));
			}

			//����˳��Ҳ�ǰ��������˳�򣬾�û����
			for (int i = 0; i < maxDepth; ++i)
			{
				for (size_t j = 0; j < scene.lights.size(); ++j)
				{
					//һ��interaction point
					//��Դ����һ��array����
					//bsdf����һ��array����
					//�ܹ�����request2DArray
					sampler.Request2DArray(nLightSamples[j]);
					sampler.Request2DArray(nLightSamples[j]);
				}
			}
		}
	}

	Spectrum DirectLightingIntegrator::Li(const RayDifferential& ray, const Scene& scene,
		Sampler& sampler, MemoryArena& arena, int depth) const
	{
		Spectrum L(0.f);
		// Find closest ray intersection or return background radiance
		Interaction isect;
		if (!scene.Intersect(ray, &isect)) 
		{
			//���û�кͳ������κε�geometry�ཻ
			//����ÿ��light escape the scene bounds
			for (const auto& light : scene.lights) 
				L += light->LiEscape(ray);
			return L;
		}

		//�����Ǽ���interaction���ϵ�material��properties
		isect.ComputeScatteringFunctions(ray, arena);

		if (!isect.bsdf)
			return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);
		Vector3f wo = isect.wo;

		// Compute emitted light if ray hit an area light source
		//first term of the light equation:
		//Lo = Le + ��f(p, wo, wi) * Li(p, wi) * |cos��i|dwi
		L += isect.Le(wo);
		if (scene.lights.size() > 0) 
		{
			// Compute direct lighting for _DirectLightingIntegrator_ integrator
			if (strategy == LightStrategy::UniformSampleAll)
				L += UniformSampleAllLights(isect, scene, arena, sampler,
					nLightSamples, false);
			else
				L += UniformSampleOneLight(isect, scene, arena, sampler);
		}
		if (depth + 1 < maxDepth) 
		{
			// Trace rays for specular reflection and refraction
			L += SpecularReflect(ray, isect, scene, sampler, arena, depth);
			L += SpecularTransmit(ray, isect, scene, sampler, arena, depth);
		}
		return L;
	}
}
