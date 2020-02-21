#pragma once

#include "spectrum.h"
#include "material.h"

namespace AIR
{
	class MatteMaterial : public Material
	{
	public:
		virtual void ComputeScatteringFunctions(Interaction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const;

	private:
		//diffuse refletion value
		std::shared_ptr<Texture<Spectrum>> Kd;

		//roughness scale value
		std::shared_ptr<Texture<Spectrum>> roughness;
	};


}
