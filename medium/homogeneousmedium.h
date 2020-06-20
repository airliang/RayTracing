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
		//����ϵ��
		const Spectrum sigma_a;
		//ɢ��ϵ��
		const Spectrum sigma_s;
		//˥��ϵ����sigma_t = sigma_a + sigma_s
		const Spectrum sigma_t;
		//phase function��gϵ��
		const Float g;
	};
}
