#pragma once

#include "spectrum.h"
#include "material.h"

namespace AIR
{
	class MatteMaterial : public Material
	{
	public:
		MatteMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
			const std::shared_ptr<Texture<Float>>& sigma,
			const std::shared_ptr<Texture<Float>>& bumpMap)
			: Kd(Kd), sigma(sigma), bumpMap(bumpMap) { }
		void ComputeScatteringFunctions(Interaction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const;

	private:
		//diffuse refletion value
		std::shared_ptr<Texture<Spectrum>> Kd;

		//roughness scale value
		std::shared_ptr<Texture<Float>> sigma, bumpMap;
	};


}
