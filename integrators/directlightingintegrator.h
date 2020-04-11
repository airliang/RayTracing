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
		DirectLightingIntegrator(LightStrategy strategy, int maxDepth,
			std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds)
			: SamplerIntegrator(camera, sampler, pixelBounds), strategy(strategy),
			maxDepth(maxDepth) { }
		Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena, int depth) const;
		void Preprocess(const Scene& scene, Sampler& sampler);

	private:
		const LightStrategy strategy;
		const int maxDepth;
		std::vector<int> nLightSamples;
	};
}
