#pragma once
#include "integrator.h"

namespace AIR
{
	//Â·¾¶»ý·ÖÆ÷
	class PathIntegrator : public SamplerIntegrator
	{
	public:
		PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds)
			: SamplerIntegrator(camera, sampler, pixelBounds), maxDepth(maxDepth) { }

		virtual Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena,
			int depth = 0) const;
	private:
		const int maxDepth;
	};
}
