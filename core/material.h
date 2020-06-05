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
		//����SurfaceInteraction�����ԣ�����interaction��Ӧ��BSDF
		//·��׷��ray��SurfaceInteraction�����ཻʱ�����øú���������bsdf������
		//si material�����ĸ�����
		//arena ��������������allocate memory for BSDFs
		//mode ������˫��
		//allowMultipleLobes 
		virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const = 0;
		
		//d��shared_ptr���������ܻ����
		static void Bump(const std::shared_ptr<Texture<Float>> &d,
			SurfaceInteraction *si);
	private:

	};

}
