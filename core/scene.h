#pragma once
#include "geometry.h"

namespace AIR
{
	class Light;
	class Scene
	{
	public:
		const Bounds3f& WorldBound() const 
		{ 
			return worldBound; 
		}

		std::vector<std::shared_ptr<Light>> lights;
	private:
		Bounds3f worldBound;
	};
}

