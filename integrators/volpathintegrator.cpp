#include "volpathintegrator.h"
#include "scene.h"
#include "light.h"
#include "interaction.h"
#include "bsdf.h"

namespace AIR
{
	void VolPathIntegrator::Preprocess(const Scene& scene, Sampler& sampler)
	{

	}

	Spectrum VolPathIntegrator::Li(const RayDifferential& r, const Scene& scene,
		Sampler& sampler, MemoryArena& arena, int depth) const
	{
		Spectrum L(0.f);
		Spectrum beta(1.f);
		RayDifferential ray(r);
		bool specularBounce = false;
		int bounces;

		//etaScale暂时不知有何用
		Float etaScale = 1;

		for (bounces = 0;; ++bounces) 
		{
			// Intersect _ray_ with scene and store intersection in _isect_
			SurfaceInteraction isect;
			//相交检测，修改ray.tMax
			bool foundIntersection = scene.Intersect(ray, &isect);

			//采样ray方向上的MediumInteraction
			MediumInteraction mi;
			if (ray.medium) 
				beta *= ray.medium->Sample(ray, sampler, arena, &mi);
			if (beta.IsBlack()) 
				break;

			if (mi.IsValid())
			{
				if (bounces >= maxDepth) 
					break;
				//采样影响这个MediumInteraction的灯光
				L += beta * UniformSampleOneLight(mi, scene, arena, sampler, true,
					nullptr);

				Vector3f wo = -ray.d, wi;
				//通过相位函数采样入射光
				mi.phase->Sample_p(wo, &wi, sampler.Get2D());
				ray = mi.SpawnRay(wi);
				specularBounce = false;
			}
			else
			{
				//采样到的是surface
				//++surfaceInteractions;

				if (bounces == 0 || specularBounce) 
				{
					// Add emitted light at path vertex or from the environment
					if (foundIntersection)
						L += beta * isect.Le(-ray.d);
					else
						for (const auto& light : scene.lights)
							L += beta * light->LiEscape(ray);
				}

				// Terminate path if ray escaped or _maxDepth_ was reached
				if (!foundIntersection || bounces >= maxDepth) 
					break;

				// Compute scattering functions and skip over medium boundaries
				isect.ComputeScatteringFunctions(ray, arena, true);
				if (!isect.bsdf) 
				{
					ray = isect.SpawnRay(ray.d);
					bounces--;
					continue;
				}

				//当前路径的采样光源
				//采样出的光源要和上次的throughput相乘
				L += beta * UniformSampleOneLight(isect, scene, arena, sampler, true, nullptr);

				// Sample BSDF to get new path direction
				Vector3f wo = -ray.d, wi;
				Float pdf;
				BxDFType flags;
				Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
					BSDF_ALL, &flags);
				if (f.IsBlack() || pdf == 0.f) 
					break;

				beta *= f * Vector3f::AbsDot(wi, isect.shading.n) / pdf;
				specularBounce = (flags & BSDF_SPECULAR) != 0;
				if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
					Float eta = isect.bsdf->eta;
					// Update the term that tracks radiance scaling for refraction
					// depending on whether the ray is entering or leaving the
					// medium.
					etaScale *= (Vector3f::Dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
				}
				ray = isect.SpawnRay(wi);
			}

			Spectrum rrBeta = beta * etaScale;
			if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) 
			{
				Float q = std::max((Float).05, 1 - rrBeta.MaxComponentValue());
				if (sampler.Get1D() < q) 
					break;
				beta /= 1 - q;

			}
		}

		return L;
	}
}
