#pragma once

#include "integrator.h"


namespace AIR
{
	class VolPathIntegrator : public SamplerIntegrator
	{
	public:
		VolPathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds, Float rrThreshold = 1)
			: SamplerIntegrator(camera, sampler, pixelBounds),
			maxDepth(maxDepth),
			rrThreshold(rrThreshold) { }
		void Preprocess(const Scene& scene, Sampler& sampler);
		Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena, int depth) const;
	private:
		const int maxDepth;
		//使用russian routine的阈值
		const Float rrThreshold;
		//const std::string lightSampleStrategy;
		//std::unique_ptr<LightDistribution> lightDistribution;
	};

}
