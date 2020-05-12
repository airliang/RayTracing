#pragma once
#include <string>
#include <vector>
#include "memory.h"
#include "geometry.h"
#include "quaternion.h"

namespace AIR
{
	class Integrator;
	class Sampler;
	class Transform;
	class Scene;
	class Integrator;
	class Camera;
	class Film;
	class Filter;
	class Light;
	class Primitive;

	//这里，我把Transform当成是Unity的GameObject
	//每个Primitive有一个Transform的指针
	//可以理解成一个Transform有一堆Primitive的集合。
	class TransformCache {
	public:
		static TransformCache& GetInstance()
		{
			static TransformCache instance;
			return instance;
		}
		

		// TransformCache Public Methods
		Transform* Lookup(const Transform& t);

		void Clear();

	private:
		TransformCache()
			: hashTable(512), hashTableOccupancy(0) {}
		void Insert(Transform* tNew);
		void Grow();

		static uint64_t Hash(const Transform& t);

		// TransformCache Private Data
		std::vector<Transform*> hashTable;
		int hashTableOccupancy;
		MemoryArena arena;
	};

	struct CameraParam
	{
		Vector3f position;
		Quaternion rotation;
		Vector3f scale;
		Bounds2f cropBounds;
		//Point2i  imageResolution;
		Float    fov;
		bool     orthogonal;
	};

	struct FilmParam
	{
		std::string imageFile;

		Point2i resolution = Point2i(512, 512);
		Bounds2f cropWindow = Bounds2f(Vector2f::zero, Vector2f::one);

	};

	struct FilterParam
	{
		std::string filterName;
		//filter的半径
		Vector2f radius;
		//gaussian filter要用的参数
		Float    gaussianAlpha;
	};

	struct SamplerParam
	{
		std::string samplerName;
		union
		{
			struct
			{
				int xSamples;
				int ySamples;
				int dimension;
				bool jitter;
			} stratified;

			struct
			{

			} halton;
		};
	};

	struct RenderOptions {
		// RenderOptions Public Methods
		Integrator* MakeIntegrator();
		Scene* MakeScene();
		Camera* MakeCamera();
		Film* MakeFilm();
		std::unique_ptr<Filter> MakeFilter();
		Sampler* MakeSampler();

		// RenderOptions Public Data
		Float transformStartTime = 0, transformEndTime = 1;
		std::string FilterName = "box";
		//ParamSet FilterParams;
		std::string FilmName = "image";
		//ParamSet FilmParams;
		std::string SamplerName = "halton";
		//ParamSet SamplerParams;
		std::string AcceleratorName = "bvh";
		//ParamSet AcceleratorParams;
		std::string IntegratorName = "path";
		//ParamSet IntegratorParams;
		//std::string CameraName = "perspective";
		CameraParam cameraParams;

		FilmParam   filmParams;
		FilterParam filterParams;
		SamplerParam samplerParams;
		//ParamSet CameraParams;
		//TransformSet CameraToWorld;
		//std::map<std::string, std::shared_ptr<Medium>> namedMedia;
		std::vector<std::shared_ptr<Light>> lights;
		std::vector<std::shared_ptr<Primitive>> primitives;
		//std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
		//std::vector<std::shared_ptr<Primitive>>* currentInstance = nullptr;
		int maxDepth = 5;
		bool haveScatteringMedia = false;

		std::string sceneFile;
	};

	class Renderer
	{
	public:
		static Renderer& GetInstance()
		{
			static Renderer instance;
			return instance;
		}

		void Init();

		void Run();

		void Cleanup();

		void ParseScene(const std::string& filename);
	protected:
	private:
		Renderer();
	};
}
