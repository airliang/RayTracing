#pragma once
#include "material.h"

namespace AIR
{
	class GlassMaterial : public Material
	{
	public:
		GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
			const std::shared_ptr<Texture<Spectrum>>& Kt,
			const std::shared_ptr<Texture<Float>>& uRoughness,
			const std::shared_ptr<Texture<Float>>& vRoughness,
			const std::shared_ptr<Texture<Float>>& index,
			const std::shared_ptr<Texture<Float>>& bumpMap,
			bool remapRoughness);

		void ComputeScatteringFunctions(Interaction* si,
			MemoryArena& arena, TransportMode mode,
			bool allowMultipleLobes) const;
	private:
		//反射比例和折射比例
		std::shared_ptr<Texture<Spectrum>> Kr, Kt;
		//各向异性粗糙度
		std::shared_ptr<Texture<Float>> uRoughness, vRoughness;
		//表面的折射率
		std::shared_ptr<Texture<Float>> index;
		std::shared_ptr<Texture<Float>> bumpMap;
		bool remapRoughness;
	};
}
