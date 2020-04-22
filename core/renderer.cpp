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
	std::shared_ptr<RObject> MakeAccelerator(
		const std::string& name,
		std::vector<std::shared_ptr<RObject>> prims
		) 
	{
		std::shared_ptr<RObject> accel;
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
		std::vector<std::shared_ptr<RObject>> primitives;
		//std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
		//std::vector<std::shared_ptr<Primitive>>* currentInstance = nullptr;
		int maxDepth = 5;
		bool haveScatteringMedia = false;
	};

	Scene* RenderOptions::MakeScene()
	{
		std::shared_ptr<RObject> accelerator =
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
			MakeFilm(), false);
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
