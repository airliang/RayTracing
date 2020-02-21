#pragma once
#include "interaction.h"
#include "texture.h"
#include "memory.h"

namespace AIR
{
	class Material
	{
	public:
		//根据Interaction的属性，生成interaction对应的BSDF
		virtual void ComputeScatteringFunctions(Interaction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const = 0;
		
		//d用shared_ptr的引用性能会更高
		static void Bump(const std::shared_ptr<Texture<Float>> &d,
			Interaction *si);
	private:

	};

}
