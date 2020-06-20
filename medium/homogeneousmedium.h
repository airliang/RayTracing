#pragma once

#include "medium.h"


namespace AIR
{
	class HomogeneousMedium : public Medium
	{
	public:
		HomogeneousMedium(const Spectrum& sigma_a, const Spectrum& sigma_s, Float g)
			: sigma_a(sigma_a),
			sigma_s(sigma_s),
			sigma_t(sigma_s + sigma_a),
			g(g) {}
		Spectrum Tr(const Ray& ray, Sampler& sampler) const;
		Spectrum Sample(const Ray& ray, Sampler& sampler, MemoryArena& arena,
			MediumInteraction* mi) const;

	private:
		// HomogeneousMedium Private Data
		//吸收系数
		const Spectrum sigma_a;
		//散射系数
		const Spectrum sigma_s;
		//衰减系数，sigma_t = sigma_a + sigma_s
		const Spectrum sigma_t;
		//phase function的g系数
		const Float g;
	};
}
