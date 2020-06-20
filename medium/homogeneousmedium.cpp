#include "homogeneousmedium.h"
#include "interaction.h"
#include <algorithm>

namespace AIR
{
	Spectrum HomogeneousMedium::Tr(const Ray& ray, Sampler& sampler) const
	{
		//根据pbrt 11.1的公式：
		//Tr(p->p') = e^(-∫[0,d]σt(p + tω, ω)dt)   //通过解微分方程得到
		//optical thicknes τ(p->p') = -∫[0,d]σt(p + tω, ω)dt
		//由于homogeneous medium的衰减系数σt是一个常数：
		//τ(p->p') = -∫[0,d]σtdt = -σtd
		//Tr(p->p') = e^(-σtd)
		//这里d = ray.tMax * ray.d.Length()
		return Exp(-sigma_t * std::min(ray.tMax * ray.d.Length(), MaxFloat));
	}

	Spectrum HomogeneousMedium::Sample(const Ray& ray, Sampler& sampler, 
		MemoryArena& arena, MediumInteraction* mi) const
	{
		//由于衰减系数对每个波长都不一样，所以这里先采样波长对应的channel
		//
		int channel = std::min((int)(sampler.Get1D() * Spectrum::nSamples), Spectrum::nSamples - 1);

		//采样t（这里是距离）
		//分布是指数分布
		//cdf = 1 - e^(-σt d)
		// d = -ln(1 - ξ) / σt
		float distance = -log(1 - sampler.Get1D()) / sigma_t[channel];

		//由于ray.d不是归一化向量，所以distance要转到tMax的范围内
		float t = std::min (distance / ray.d.Length(), ray.tMax);

		bool sampledMedium = t < ray.tMax;
		if (sampledMedium)
			*mi = MediumInteraction(ray(t), -ray.d, ray.time, this,
				ARENA_ALLOC(arena, HenyeyGreenstein)(g));
		else
		{
			int a = 0;
		}

		// Compute the transmittance and sampling density
		//当前样本采样后的透过率Tr
		//Tr = e^(-σt * t)
		Spectrum Tr = Exp(-sigma_t * std::min(t, MaxFloat) * ray.d.Length());

		//求pdf函数
		//pdf = cdf的求导
		//pdf(t) = σt e^(-σt t)
		//由于每个channel的pdf不一样，这里取平均值
		//pdf(t) = 1/n∑(σti * e^(-σtit)) = 1/n∑(σti * Tr_i)

		//采样到Surface的概率是
		//   1 - ∫[0, t_max]pdf(t)dt
		// = 1 - ∫[0, t_max]sigma_t * e^(-sigma_t * t)dt
		// = 1 - sigma_t * ∫[0, t_max]e^(-sigma_t * t)dt
		// = 1 - (1 - e^(-sigma_t * t)) = e^(-sigma_t * t)
		Spectrum pdfAll = sampledMedium ? Tr * sigma_t : Tr;
		float pdf = 0;
		for (int i = 0; i < Spectrum::nSamples; ++i)
		{
			pdf += pdfAll[i];
		}
		pdf /= Spectrum::nSamples;
		if (pdf == 0) {
			//CHECK(Tr.IsBlack());
			pdf = 1;
		}

		//到达该介质的光照要乘以衰减系数σs，到达ray.o的光照是要再乘以Tr
		//Lo = Li * σs * Tr;
		Spectrum beta_med = Tr * sigma_s / pdf;
		//当发射点不是介质时，不需要乘以衰减系数σs
		Spectrum beta_surf = Tr / pdf;
		return sampledMedium ? beta_med : beta_surf;
	}
}
