#pragma once
#include "interaction.h"
#include "texture.h"
#include "memory.h"

namespace AIR
{
	template <typename T>
	class Texture;

	class Material
	{
	public:
		//根据Interaction的属性，生成interaction对应的BSDF
		//路径追踪ray和Interaction发生相交时，调用该函数来决定bsdf的数据
		//si material属于哪个表面
		//arena 在其生命周期内allocate memory for BSDFs
		//mode 单向还是双向
		//allowMultipleLobes 
		virtual void ComputeScatteringFunctions(Interaction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const = 0;
		
		//d用shared_ptr的引用性能会更高
		static void Bump(const std::shared_ptr<Texture<Float>> &d,
			Interaction *si);
	private:

	};

}
