#include "light.h"


namespace AIR
{
	bool VisibilityTester::Unoccluded(const Scene& scene) const
	{
		return false;
	}

	Spectrum VisibilityTester::Tr(const Scene& scene, Sampler& sampler) const
	{
		return Spectrum(0.0f);
	}
}