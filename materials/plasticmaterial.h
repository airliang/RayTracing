#pragma once
#include "material.h"

namespace AIR
{
	class Texture;
	//a mixture of diffuse and glossy scattering
	//kd and ks to control the paticular colors and specular highlight size
	//
	class PlasticMaterial : public Material
	{
	public:
		PlasticMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
			const std::shared_ptr<Texture<Spectrum>>& Ks,
			const std::shared_ptr<Texture<Float>>& roughness,
			const std::shared_ptr<Texture<Float>>& bumpMap,
			bool remapRoughness)
			: Kd(Kd), Ks(Ks), roughness(roughness), bumpMap(bumpMap),
			remapRoughness(remapRoughness) { }

		void ComputeScatteringFunctions(Interaction* si, MemoryArena& arena,
			TransportMode mode, bool allowMultipleLobes) const;
	private:
		std::shared_ptr<Texture<Spectrum>> kd, ks;
		std::shared_ptr<Texture<Float>> roughness;
		std::shared_ptr<Texture<Float>> bumpMap;
		//if remapRoughness is true
		//roughness should vary from zero to one(clamp to [0, 1])
		const bool remapRoughness;
	};
}
