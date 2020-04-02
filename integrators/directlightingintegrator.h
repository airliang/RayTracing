#pragma once
#include "integrator.h"

namespace AIR
{
	// LightStrategy Declarations
	enum class LightStrategy 
	{ 
		//遍历全部光源，并根据光源的nSamples数量来采样。
		UniformSampleAll, 
		//只随机地取一个光源一个sample
		UniformSampleOne 
	};

	class DirectLightingIntegrator : public SamplerIntegrator
	{
	public:

	};
}
