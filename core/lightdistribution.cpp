#include "lightdistribution.h"
#include "integrator.h"
#include "scene.h"

namespace AIR
{
	LightDistribution::~LightDistribution() {}

	std::unique_ptr<LightDistribution> CreateLightSampleDistribution(
		const std::string& name, const Scene& scene) 
	{
		if (name == "uniform" || scene.lights.size() == 1)
			return std::unique_ptr<LightDistribution>
		{
			new UniformLightDistribution(scene)
		};
		else if (name == "power")
			return std::unique_ptr<LightDistribution>
		{
			new PowerLightDistribution(scene)
		};
		else if (name == "spatial")
			return std::unique_ptr<LightDistribution>{
			nullptr
		};
		else 
		{

			return std::unique_ptr<LightDistribution>{
				new UniformLightDistribution(scene)};
		}
	}

	UniformLightDistribution::UniformLightDistribution(const Scene& scene) {
		std::vector<Float> prob(scene.lights.size(), Float(1));
		distrib.reset(new Distribution1D(&prob[0], int(prob.size())));
	}

	const Distribution1D* UniformLightDistribution::Lookup(const Point3f& p) const {
		return distrib.get();
	}

	PowerLightDistribution::PowerLightDistribution(const Scene& scene)
		: distrib(ComputeLightPowerDistribution(scene)) 
	{
	}

	const Distribution1D* PowerLightDistribution::Lookup(const Point3f& p) const {
		return distrib.get();
	}

	
}
