#pragma once
#include "material.h

namespace AIR
{
	//modeled with perfect specular reflection
	class MirrorMaterial : public Material
	{
	public:
		MirrorMaterial(const std::shared_ptr<Texture<Spectrum>>& r,
			const std::shared_ptr<Texture<Float>>& bump) : Kr(r)
			, bumpMap(bumpMap)
		{

		}

		void ComputeScatteringFunctions(Interaction* si, MemoryArena& arena,
			TransportMode mode,
			bool allowMultipleLobes) const;
	protected:
	private:
		std::shared_ptr<Texture<Spectrum>> Kr;
		std::shared_ptr<Texture<Float>> bumpMap;
	};
}
