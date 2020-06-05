#include "pathintegrator.h"
#include "scene.h"
#include "light.h"
#include "interaction.h"
#include "bsdf.h"

namespace AIR
{
	//L(p1->p0) = ��[i=1,��]P(pi)
	//P(pn) = �ҡ�����(n-1����)Le(pn -> pn-1)T(pn)dA(p2)����dA(pn)
	//T(pn) = ��[j=1, n-1]f(p_i+1 -> p_i -> p_i-1)G(p_i+1, p_i)
	//beta is the path throughput weight
	//                   f(p_j+1 -> p_j -> p_j-1)|cos��j|
	//beta = ��[j=1, i-2]--------------------------------
	//                             pw(p_j+1 - p_j)
	Spectrum PathIntegrator::Li(const RayDifferential& r, const Scene& scene,
		Sampler& sampler, MemoryArena& arena, int depth) const
	{

		Spectrum L(0.f);
		
		Spectrum beta(1.f);
		
		RayDifferential ray(r);
		bool specularBounce = false;

		//bounces�����壺
		//forѭ��Ϊ��û�����ֵ��bounce > 3���ö���˹����ȥ�ж�·���Ƿ�Ҫ������ȥ
		//һ��bounceҪ�Ѹô�bounce��radiance���׼ӵ��ϴεĹ�����
		for (int bounces = 0; ; ++bounces)
		{
			SurfaceInteraction isect;
			bool foundIntersection = scene.Intersect(ray, &isect);

			//bounces == 0ʱ��
			//P(p1) = Le(p1->p0)
			//bounces == 1ʱ
			//P(p2) = ��Le(p2->p1) * f(p2->p1->p0)G(p2,p1)dA(p2)
			if (bounces == 0 || specularBounce)
			{
				//bounces = 0��ʱ�����isect��emit�Ž��м���
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

			//��ǰ·���Ĳ�����Դ
			//�������Ĺ�ԴҪ���ϴε�throughput���
			L += beta * UniformSampleOneLight(isect, scene, arena, sampler);

			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDFType flags;
			//��ǰ·������bsdf�ļ�ӹ�
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(),
				&pdf, BSDF_ALL, &flags);
			if (f.IsBlack() || pdf == 0.f)
				break;
			beta *= f * Vector3f::AbsDot(wi, isect.shading.n) / pdf;
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			ray = isect.SpawnRay(wi);

			//pbrt��·����ʽ��
			//P = P(p1) + P(p2) + P(p3) + 1/(1 - q)��[i=4,��]P(pi)
			//����bounces > 3�ſ�ʼ��Russian roulette
			if (bounces > 3) 
			{
				//beta.y��Сʱ��˵��·���Ĺ���ֵ��С
				//��ֹ�ĸ���q������
				Float q = std::max((Float).05, 1 - beta.y());
				if (sampler.Get1D() < q)
					break;
				beta /= 1 - q;
			}
		}

		return L;
	}
}
