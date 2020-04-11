#pragma once
#include "integrator.h"

namespace AIR
{
	// LightStrategy Declarations
	enum class LightStrategy 
	{ 
		//����ȫ����Դ�������ݹ�Դ��nSamples������������
		UniformSampleAll, 
		//ֻ�����ȡһ����Դһ��sample
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
