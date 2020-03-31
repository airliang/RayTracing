#pragma once
#include "geometry.h"

namespace AIR
{
	class Scene
	{
	public:
		const Bounds3f& WorldBound() const 
		{ 
			return worldBound; 
		}
	private:
		Bounds3f worldBound;
	};
}

