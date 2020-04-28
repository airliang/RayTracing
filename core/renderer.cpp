#include "renderer.h"
#include "parallelism.h"
#include "spectrum.h"
#include "scene.h"
#include "camera.h"
#include "bvhaccel.h"
#include "pathintegrator.h"
#include "film.h"
#include "boxfilter.h"
#include "gaussianfilter.h"
#include "trianglefilter.h"
#include "stratified.h"
#include "sceneparser.h"

namespace AIR
{
	static int nTransformCacheLookups = 0;
	static int nTransformCacheHits = 0;
	static size_t transformCacheBytes = 0;
	Transform* TransformCache::Lookup(const Transform& t)
	{
		++nTransformCacheLookups;

		int offset = Hash(t) & (hashTable.size() - 1);
		int step = 1;
		while (true) 
		{
			// Keep looking until we find the Transform or determine that
			// it's not present.
			if (!hashTable[offset] || *hashTable[offset] == t)
				break;
			// Advance using quadratic probing.
			offset = (offset + step * step) & (hashTable.size() - 1);
			++step;
		}
		//ReportValue(transformCacheProbes, step);
		Transform* tCached = hashTable[offset];
		if (tCached)
			++nTransformCacheHits;
		else 
		{
			tCached = arena.Alloc<Transform>();
			*tCached = t;
			Insert(tCached);
		}
		return tCached;
	}

	void TransformCache::Clear()
	{
		transformCacheBytes += arena.TotalAllocated() + hashTable.size() * sizeof(Transform*);
		hashTable.clear();
		hashTable.resize(512);
		hashTableOccupancy = 0;
		arena.Reset();
	}

	uint64_t TransformCache::Hash(const Transform& t) 
	{
		const char* ptr = (const char*)(&t.LocalToWorld());
		size_t size = sizeof(Matrix4x4);
		uint64_t hash = 14695981039346656037ull;
		while (size > 0) 
		{
			hash ^= *ptr;
			hash *= 1099511628211ull;
			++ptr;
			--size;
		}
		return hash;
	}

	void TransformCache::Insert(Transform* tNew) 
	{
		if (++hashTableOccupancy == hashTable.size() / 2)
			Grow();

		int baseOffset = Hash(*tNew) & (hashTable.size() - 1);
		for (int nProbes = 0;; ++nProbes) 
		{
			// Quadratic probing.
			int offset = (baseOffset + nProbes / 2 + nProbes * nProbes / 2) & (hashTable.size() - 1);
			if (hashTable[offset] == nullptr) 
			{
				hashTable[offset] = tNew;
				return;
			}
		}
	}

	void TransformCache::Grow() {
		std::vector<Transform*> newTable(2 * hashTable.size());
		//LOG(INFO) << "Growing transform cache hash table to " << newTable.size();

		// Insert current elements into newTable.
		for (Transform* tEntry : hashTable) {
			if (!tEntry) continue;

			int baseOffset = Hash(*tEntry) & (hashTable.size() - 1);
			for (int nProbes = 0;; ++nProbes) {
				// Quadratic probing.
				int offset = (baseOffset + nProbes / 2 + nProbes * nProbes / 2) & (hashTable.size() - 1);
				if (newTable[offset] == nullptr) {
					newTable[offset] = tEntry;
					break;
				}
			}
		}

		std::swap(hashTable, newTable);
	}

	std::shared_ptr<Primitive> MakeAccelerator(
		const std::string& name,
		std::vector<std::shared_ptr<Primitive>> prims
		) 
	{
		std::shared_ptr<Primitive> accel;
		if (name == "bvh")
			accel = BVHAccel::CreateBVHAccelerator(std::move(prims));
		else if (name == "kdtree")
			accel = nullptr;
		else
		{
			//Warning("Accelerator \"%s\" unknown.", name.c_str());
		}
			
		//paramSet.ReportUnused();
		return accel;
	}

	struct CameraParam
	{
		Vector3f position;
		Quaternion rotation;
		Vector3f scale;
		Bounds2f cropBounds;
		Point2i  imageResolution;
		Float    fov;
		bool     orthogonal;
	};

	struct FilmParam
	{
		std::string imageFile;
		
		Point2i resolution;
		Bounds2f cropWindow;

	};

	struct FilterParam
	{
		std::string filterName;
		Vector2f radius;
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
		Integrator* MakeIntegrator() const;
		Scene* MakeScene();
		Camera* MakeCamera() const;
		Film* MakeFilm() const;
		std::unique_ptr<Filter> MakeFilter() const;
		Sampler* MakeSampler() const;

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
	};

	Scene* RenderOptions::MakeScene()
	{
		std::shared_ptr<Primitive> accelerator =
			MakeAccelerator(AcceleratorName, std::move(primitives));
		if (!accelerator) 
			accelerator = std::make_shared<BVHAccel>(primitives);
		Scene* scene = new Scene(accelerator, lights);
		// Erase primitives and lights from _RenderOptions_
		primitives.clear();
		lights.clear();
		return scene;
	}

	Integrator* RenderOptions::MakeIntegrator() const
	{
		std::shared_ptr<Camera> camera(MakeCamera());
		std::shared_ptr<Sampler> sampler(MakeSampler());
		Integrator* integrator = nullptr;
		Bounds2i pixelBounds = camera->film->GetOutputSampleBounds();
		if (IntegratorName == "path")
		{
			integrator = new PathIntegrator(maxDepth, camera, sampler, pixelBounds);
		}
		return integrator;
	}

	Camera* RenderOptions::MakeCamera() const
	{
		return Camera::CreateCamera(Transform(cameraParams.position, cameraParams.rotation, cameraParams.scale),
			cameraParams.cropBounds,
			MakeFilm(), cameraParams.fov, cameraParams.orthogonal);
	}

	Film* RenderOptions::MakeFilm() const
	{
		std::unique_ptr<Filter> filter = MakeFilter();
		return new Film(filmParams.resolution, filmParams.cropWindow, std::move(filter), filmParams.imageFile);
	}

	std::unique_ptr<Filter> RenderOptions::MakeFilter() const
	{
		Filter* filter = nullptr;
		if (filterParams.filterName == "gaussian")
		{
			filter = new GaussianFilter(filterParams.radius, filterParams.gaussianAlpha);
		}
		else if (filterParams.filterName == "triangle")
		{
			filter = new TriangleFilter(filterParams.radius);
		}
		else
		{
			filter = new BoxFilter(filterParams.radius);
		}

		return std::unique_ptr<Filter>(filter);
	}

	Sampler* RenderOptions::MakeSampler() const
	{
		Sampler* sampler = nullptr;
		if (samplerParams.samplerName == "stratified")
		{
			sampler = new StratifiedSampler(samplerParams.stratified.xSamples, samplerParams.stratified.ySamples,
				samplerParams.stratified.jitter, samplerParams.stratified.dimension);
		}

		return sampler;
	}

	RenderOptions g_renderOptions;

	Renderer::Renderer()
	{
		
	}

	void Renderer::ParseScene(const std::string& filename)
	{
		SceneParser parser;
		parser.Load(filename, g_renderOptions.lights, g_renderOptions.primitives);

		g_renderOptions.cameraParams.cropBounds = Bounds2f(Point2f(0, 0), Point2f(1, 1));
		g_renderOptions.cameraParams.fov = parser.GetCameraFOV();
		g_renderOptions.cameraParams.orthogonal = parser.IsCameraOrtho();
		g_renderOptions.cameraParams.position = parser.GetCameraTransform()->Position();
		g_renderOptions.cameraParams.rotation = parser.GetCameraTransform()->Rotation();
		g_renderOptions.cameraParams.scale = Vector3f::one;
		g_renderOptions.cameraParams.imageResolution = Point2i(800, 600);
	}

	void Renderer::Init()
	{
		SampledSpectrum::Init();
		ParallelInit();
	}

	void Renderer::Cleanup()
	{
		ParallelCleanup();
	}

	void Renderer::Run()
	{
		std::unique_ptr<Integrator> integrator(g_renderOptions.MakeIntegrator());
		std::unique_ptr<Scene> scene(g_renderOptions.MakeScene());

		if (scene && integrator)
			integrator->Render(*scene.get());
	}
}
