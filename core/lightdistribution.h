#pragma once
#include "geometry.h"
#include "sampling.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace AIR
{
	class Scene;
	class LightDistribution 
	{
	public:
		virtual ~LightDistribution();

		// Given a point |p| in space, this method returns a (hopefully
		// effective) sampling distribution for light sources at that point.
		virtual const Distribution1D* Lookup(const Point3f& p) const = 0;
	};

	std::unique_ptr<LightDistribution> CreateLightSampleDistribution(
		const std::string& name, const Scene& scene);

	// The simplest possible implementation of LightDistribution: this returns
	// a uniform distribution over all light sources, ignoring the provided
	// point. This approach works well for very simple scenes, but is quite
	// ineffective for scenes with more than a handful of light sources. (This
	// was the sampling method originally used for the PathIntegrator and the
	// VolPathIntegrator in the printed book, though without the
	// UniformLightDistribution class.)
	class UniformLightDistribution : public LightDistribution {
	public:
		UniformLightDistribution(const Scene& scene);
		const Distribution1D* Lookup(const Point3f& p) const;

	private:
		std::unique_ptr<Distribution1D> distrib;
	};

	// PowerLightDistribution returns a distribution with sampling probability
	// proportional to the total emitted power for each light. (It also ignores
	// the provided point |p|.)  This approach works well for scenes where
	// there the most powerful lights are also the most important contributors
	// to lighting in the scene, but doesn't do well if there are many lights
	// and if different lights are relatively important in some areas of the
	// scene and unimportant in others. (This was the default sampling method
	// used for the BDPT integrator and MLT integrator in the printed book,
	// though also without the PowerLightDistribution class.)
	class PowerLightDistribution : public LightDistribution {
	public:
		PowerLightDistribution(const Scene& scene);
		const Distribution1D* Lookup(const Point3f& p) const;

	private:
		std::unique_ptr<Distribution1D> distrib;
	};
}
