#pragma once
#include "interaction.h"
#include "texture.h"
#include "memory.h"

namespace AIR
{
	class Material
	{
	public:
		//����Interaction�����ԣ�����interaction��Ӧ��BSDF
		virtual void ComputeScatteringFunctions(Interaction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const = 0;
		
		//d��shared_ptr���������ܻ����
		static void Bump(const std::shared_ptr<Texture<Float>> &d,
			Interaction *si);
	private:

	};

}
