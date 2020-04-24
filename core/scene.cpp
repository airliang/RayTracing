#include "scene.h"
#include "robject.h"

namespace AIR
{
	Scene::Scene(std::shared_ptr<Primitive> accel, const std::vector<std::shared_ptr<Light>>& lights)
		: aggregate(accel), lights(lights)
	{
		worldBound = aggregate->WorldBound();
	}

	bool Scene::Intersect(const Ray& ray, Interaction* isect) const {
		//++nIntersectionTests;
		//DCHECK_NE(ray.d, Vector3f(0, 0, 0));
		return aggregate->Intersect(ray, isect);
	}

	bool Scene::IntersectP(const Ray& ray) const {
		//++nShadowTests;
		//DCHECK_NE(ray.d, Vector3f(0, 0, 0));
		return aggregate->IntersectP(ray);
	}

	bool Scene::IntersectTr(Ray ray, Sampler& sampler, Interaction* isect,
		Spectrum* Tr) const {
		*Tr = Spectrum(1.f);
		while (true) {
			bool hitSurface = Intersect(ray, isect);
			// Accumulate beam transmittance for ray segment
			//if (ray.medium) 
			//	*Tr *= ray.medium->Tr(ray, sampler);

			// Initialize next ray segment or terminate transmittance computation
			if (!hitSurface) return false;
			if (isect->robject->GetMaterial() != nullptr)
				return true;
			ray = isect->SpawnRay(ray.d);
		}
		return false;
	}
}
