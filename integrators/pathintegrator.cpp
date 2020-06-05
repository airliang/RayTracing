#include "pathintegrator.h"
#include "scene.h"
#include "light.h"
#include "interaction.h"
#include "bsdf.h"

namespace AIR
{
	//L(p1->p0) = ∑[i=1,∞]P(pi)
	//P(pn) = ∫……∫(n-1个∫)Le(pn -> pn-1)T(pn)dA(p2)……dA(pn)
	//T(pn) = ∏[j=1, n-1]f(p_i+1 -> p_i -> p_i-1)G(p_i+1, p_i)
	//beta is the path throughput weight
	//                   f(p_j+1 -> p_j -> p_j-1)|cosθj|
	//beta = ∏[j=1, i-2]--------------------------------
	//                             pw(p_j+1 - p_j)
	Spectrum PathIntegrator::Li(const RayDifferential& r, const Scene& scene,
		Sampler& sampler, MemoryArena& arena, int depth) const
	{

		Spectrum L(0.f);
		
		Spectrum beta(1.f);
		
		RayDifferential ray(r);
		bool specularBounce = false;

		//bounces的意义：
		//for循环为何没有最大值？bounce > 3后用俄罗斯轮盘去判断路径是否要计算下去
		//一次bounce要把该次bounce的radiance贡献加到上次的贡献里
		for (int bounces = 0; ; ++bounces)
		{
			SurfaceInteraction isect;
			bool foundIntersection = scene.Intersect(ray, &isect);

			//bounces == 0时，
			//P(p1) = Le(p1->p0)
			//bounces == 1时
			//P(p2) = ∫Le(p2->p1) * f(p2->p1->p0)G(p2,p1)dA(p2)
			if (bounces == 0 || specularBounce)
			{
				//bounces = 0的时候才算isect的emit才进行计算
				if (!foundIntersection)
				{
					for (const auto& light : scene.lights)
						L += beta * light->LiEscape(ray);
					return L;
				}
				else
					L += beta * isect.Le(-ray.d);
			}
			
			if (!foundIntersection || bounces >= maxDepth)
			{
				break;
			}

			isect.ComputeScatteringFunctions(ray, arena, true);
			if (!isect.bsdf) 
			{
				ray = isect.SpawnRay(ray.d);
				bounces--;
				continue;
			}

			//当前路径的采样光源
			//采样出的光源要和上次的throughput相乘
			L += beta * UniformSampleOneLight(isect, scene, arena, sampler);

			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDFType flags;
			//当前路径采样bsdf的间接光
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(),
				&pdf, BSDF_ALL, &flags);
			if (f.IsBlack() || pdf == 0.f)
				break;
			beta *= f * Vector3f::AbsDot(wi, isect.shading.n) / pdf;
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			ray = isect.SpawnRay(wi);

			//pbrt的路径公式：
			//P = P(p1) + P(p2) + P(p3) + 1/(1 - q)∑[i=4,∞]P(pi)
			//所以bounces > 3才开始用Russian roulette
			if (bounces > 3) 
			{
				//beta.y很小时，说明路径的贡献值很小
				//终止的概率q就增大
				Float q = std::max((Float).05, 1 - beta.y());
				if (sampler.Get1D() < q)
					break;
				beta /= 1 - q;
			}
		}

		return L;
	}
}
