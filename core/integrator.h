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
	class Interaction;

	//�ڴ���һ��pixel��sampler��ʱ��(һ��pixel��sampler�ж������)
	//������������light,������Ⱦ���̼�������light�Ļ���
	//������bsdf�����pixel sampler��radiance��
	//���賡����n��light����ʽ���£�
	//��[1,n]��f(p,w0,wi)Ld(j)(p,wi)|cos��i|dwi
	//����Ld(j)���� incident radiance from jth light
	//it camera�����ray��geometry�Ľ���
	//scene��ǰ����
	//arena �ڴ������
	//nLightSamples ÿ��light��Light::nSamples���������鳤����scene���light������
	Spectrum UniformSampleAllLights(const Interaction& it,
		const Scene& scene, MemoryArena& arena, Sampler& sampler,
		const std::vector<int>& nLightSamples, bool handleMedia);

	//����������ڴ�����Դ��ʱ�򣬲��Ƽ�ʹ�ñ�������light�ķ���
	//����Ҫ����f(x) + g(x),
	//E[f(x) + g(x)] = [��f(x)*p + g(x)*(1-p)] * 2
	//�ڴ���һ��pixel��sampler��ʱ��(һ��pixel��sampler�ж������)
	//���þ�������������ѡ��scene���һ��light
	//���������溯��һ��
	//lightDistrib 
	Spectrum UniformSampleOneLight(const Interaction& it, const Scene& scene,
		MemoryArena& arena, Sampler& sampler,
		bool handleMedia = false,
		const Distribution1D* lightDistrib = nullptr);

	//����һ����Դ�Ե�it�����乱��
	//it ������Ľ���
	//uShading ����bsdf���������
	//light ����������light
	//uLight ������Դ���������
	//scene sampler arena handleMedia ͬ����ĺ���
	//specular �Ƿ�ֻ����perfectly specular lobe
	Spectrum EstimateDirect(const Interaction& it, const Point2f& uShading,
		const Light& light, const Point2f& uLight,
		const Scene& scene, Sampler& sampler,
		MemoryArena& arena, bool handleMedia = false,
		bool specular = false);

	//����������
	//pbrt�У������������radiance����ͨ��bsdf�������
	//bsdf���ɵĳ����radianceҲ��ͨ��������������
	//�����Ҫ�������������
	class Integrator
	{
	public:
		virtual ~Integrator(){}
		//��Ⱦһ������
		//ʵ������������Ⱦ��image�������
		virtual void Render(const Scene& scene) = 0;
	};

	//ͨ��Sampler�ϵ�һϵ�����������image�ϵĵ㣩�������
	//��������ΪSamplerIntegrator
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
		//sampler ����0-1��������Ĳ�����
		//arena �ڴ��������ֻ���ڵ�ǰray�ļ�����̣���radiance������ϣ�
		//�ɸù�����������ڴ�Ҫ�����ͷ� 
		//depth ray�ķ�������
		virtual Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena,
			int depth = 0) const = 0;

		Spectrum SpecularReflect(const RayDifferential& ray, const SurfaceInteraction& isect,
			const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const;

		Spectrum SpecularTransmit(const RayDifferential& ray, const SurfaceInteraction& isect,
			const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const;
	protected:
		// SamplerIntegrator Protected Data
		std::shared_ptr<const Camera> camera;

	private:
		// SamplerIntegrator Private Data
		//�����������ͣ�������halton��������stratified
		std::shared_ptr<Sampler> sampler;
		const Bounds2i pixelBounds;

	};
}
