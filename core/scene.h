#pragma once
#include "geometry.h"

namespace AIR
{
	class Light;
	class Interaction;
	class RObject;
	class Scene
	{
	public:
		Scene(std::shared_ptr<RObject>& accel, const std::vector<std::shared_ptr<Light>>& lights);
		const Bounds3f& WorldBound() const 
		{ 
			return worldBound; 
		}

		bool Intersect(const Ray& ray, Interaction* isect) const;
		bool IntersectP(const Ray& ray) const;
		bool IntersectTr(Ray ray, Sampler& sampler, Interaction* isect,
			Spectrum* transmittance) const;

		std::vector<std::shared_ptr<Light>> lights;
	private:
		std::shared_ptr<RObject> aggregate;
		Bounds3f worldBound;
	};
}

