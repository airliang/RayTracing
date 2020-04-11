#pragma once
#include "camera.h"
#include "memory.h"
#include "sampler.h"
#include "spectrum.h"


namespace AIR
{
	class Scene;
	class Distribution1D;
	class Light;

	//在处理一个pixel的sampler的时候，(一个pixel的sampler有多个样本)
	//遍历场景所有light,根据渲染方程计算所有light的积分
	//并根据bsdf计算改pixel sampler的radiance。
	//假设场景有n个light，公式如下：
	//∑[1,n]∫f(p,w0,wi)Ld(j)(p,wi)|cosθi|dwi
	//其中Ld(j)代表 incident radiance from jth light
	//it camera发射的ray与geometry的交点
	//scene当前场景
	//arena 内存分配器
	//nLightSamples 每个light的Light::nSamples数量，数组长度是scene里的light的数量
	Spectrum UniformSampleAllLights(const Interaction& it,
		const Scene& scene, MemoryArena& arena, Sampler& sampler,
		const std::vector<int>& nLightSamples, bool handleMedia);

	//如果场景存在大量光源的时候，不推荐使用遍历所有light的方法
	//假如要估计f(x) + g(x),
	//E[f(x) + g(x)] = [∑f(x)*p + g(x)*(1-p)] * 2
	//在处理一个pixel的sampler的时候，(一个pixel的sampler有多个样本)
	//利用均匀随机数随机地选择scene里的一个light
	//参数和上面函数一致
	//lightDistrib 
	Spectrum UniformSampleOneLight(const Interaction& it, const Scene& scene,
		MemoryArena& arena, Sampler& sampler,
		bool handleMedia = false,
		const Distribution1D* lightDistrib = nullptr);

	//估计一个光源对点it的入射贡献
	//it 光入射的交点
	//uShading 采样bsdf的随机变量
	//light 产生入射光的light
	//uLight 采样光源的随机变量
	//scene sampler arena handleMedia 同上面的函数
	//specular 是否只考虑perfectly specular lobe
	Spectrum EstimateDirect(const Interaction& it, const Point2f& uShading,
		const Light& light, const Point2f& uLight,
		const Scene& scene, Sampler& sampler,
		MemoryArena& arena, bool handleMedia = false,
		bool specular = false);

	//积分器基类
	//pbrt中，到达摄像机的radiance都是通过bsdf计算出来
	//bsdf生成的出射光radiance也是通过积分入射光求出
	//因此需要积分器来求积分
	class Integrator
	{
	public:
		virtual ~Integrator(){}
		//渲染一个场景
		//实现他可以是渲染到image里或其他
		virtual void Render(const Scene& scene) = 0;
	};

	//通过Sampler上的一系列样本（输出image上的点）来求积分
	//所以命名为SamplerIntegrator
	class SamplerIntegrator : public Integrator
	{
	public:
		SamplerIntegrator(std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds)
			: camera(camera), sampler(sampler), pixelBounds(pixelBounds) {}
		virtual void Preprocess(const Scene& scene, Sampler& sampler) {}
		void Render(const Scene& scene);

		//the method compute the radiance arriving at the film
		//ray camera spawn ray
		//scene the scene to be rendered
		//sampler 生成0-1随机样本的采样器
		//arena 内存管理器，只用于当前ray的计算过程，当radiance计算完毕，
		//由该管理器分配的内存要立即释放 
		//depth ray的反弹次数
		virtual Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena,
			int depth = 0) const = 0;

	protected:
		// SamplerIntegrator Protected Data
		std::shared_ptr<const Camera> camera;

	private:
		// SamplerIntegrator Private Data
		//采样器的类型，可以是halton，可以是stratified
		std::shared_ptr<Sampler> sampler;
		const Bounds2i pixelBounds;

	};
}
