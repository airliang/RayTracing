#include "light.h"
#include "scene.h"
#include "robject.h"

namespace AIR
{
	bool VisibilityTester::Unoccluded(const Scene& scene) const
	{
		return !scene.IntersectP(p0.SpawnRayTo(p1));
	}

	Spectrum VisibilityTester::Tr(const Scene& scene, Sampler& sampler) const
	{
		Ray ray(p0.SpawnRayTo(p1));
		Spectrum Tr(1.f);
		while (true) {
			Interaction isect;
			bool hitSurface = scene.Intersect(ray, &isect);
			// Handle opaque surface along ray's path
			if (hitSurface && isect.primitive->GetMaterial() != nullptr)
				return Spectrum(0.0f);

			// Update transmittance for current ray segment
			//if (ray.medium) 
			//	Tr *= ray.medium->Tr(ray, sampler);

			// Generate next ray segment or return final transmittance
			if (!hitSurface) break;
			ray = isect.SpawnRayTo(p1);
		}
		return Tr;
	}
}