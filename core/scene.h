#pragma once
#include "geometry.h"
#include "spectrum.h"

namespace AIR
{
	class Light;
	class SurfaceInteraction;
	class Primitive;
	class Sampler;
	//class Spectrum;

	class Scene
	{
	public:
		Scene(std::shared_ptr<Primitive> accel, const std::vector<std::shared_ptr<Light>>& lights);
		const Bounds3f& WorldBound() const 
		{ 
			return worldBound; 
		}

		bool Intersect(const Ray& ray, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;
		bool IntersectTr(Ray ray, Sampler& sampler, SurfaceInteraction* isect,
			Spectrum* transmittance) const;

		std::vector<std::shared_ptr<Light>> lights;
	private:
		std::shared_ptr<Primitive> aggregate;
		Bounds3f worldBound;
	};
}

