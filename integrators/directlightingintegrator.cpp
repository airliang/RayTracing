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

			//遍历顺序也是按这个生成顺序，就没问题
			for (int i = 0; i < maxDepth; ++i)
			{
				for (size_t j = 0; j < scene.lights.size(); ++j)
				{
					//一个interaction point
					//光源采样一个array数组
					//bsdf采样一个array数组
					//总共两次request2DArray
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
		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect)) 
		{
			//如果没有和场景中任何的geometry相交
			//返回每个light escape the scene bounds
			for (const auto& light : scene.lights) 
				L += light->LiEscape(ray);
			return L;
		}

		//这里是计算interaction点上的material的properties
		isect.ComputeScatteringFunctions(ray, arena);

		if (!isect.bsdf)
			return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);
		Vector3f wo = isect.wo;

		// Compute emitted light if ray hit an area light source
		//first term of the light equation:
		//Lo = Le + ∫f(p, wo, wi) * Li(p, wi) * |cosθi|dwi
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
