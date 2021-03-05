// RayTracing.cpp: 定义应用程序的入口点。
//

#include "RayTracing.h"
#include "matrix.h"
#include "quaternion.h"
#include "sphere.h"
#include "geometryparam.h"
#include "film.h"
#include "boxfilter.h"
#include "parallelism.h"
#include "renderer.h"
#include "runoption.h"
#include "fileutil.h"
#include "log.h"
#include "imageio.h"
//#include <filesystem>h"
#include "spdlog/spdlog.h"
#include "cpptutorial.h"
#include "haltonsampler.h"
using namespace std;
using namespace AIR;
int main(int argc, char* argv[])
{
	Log::Init();
	Log::Info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	int testLogVar = 1000;
	Log::Error("HaltonSampler can only sample {}{} ",
		"dimensions.", testLogVar);
	
	test_showbytes();
	//test_rightvalue();
	test_operators();

	Log::Info("sizeof SamplerState={}", sizeof(SamplerState));
	//system("pause");
	int row = 9 % 4;
	int col = 9 / 4;

	std::string directory = DirectoryContaining("scene.rt");

	//这里获取全路径
	std::string rootPath = argv[0];//std::filesystem::current_path();
	int index = rootPath.find("bin");
	rootPath = rootPath.substr(0, index);
	Log::Info("Application root path is:{}", rootPath);

	//return 0;

	Transform transform;
	std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(1.0f, -1.0f, 1.0f, 2.0f * Pi, &transform);
	Ray ray(Vector3f(0, 0, 2.0f), -Vector3f::forward);
	SurfaceInteraction interaction;
	Float tHit;
	sphere->Intersect(ray, &tHit, &interaction);


	GlobalOptions options;
	std::vector<std::string> filenames;

	for (int i = 1; i < argc; ++i)
	{
		if (!strncmp(argv[i], "--nthreads=", 11)) 
		{
			options.nThreads = atoi(&argv[i][11]);
		}
		else if (!strncmp(argv[i], "-w", 2))
		{
			options.filmWidth = atoi(argv[++i]);
		}
		else if (!strncmp(argv[i], "-h", 2))
		{
			options.filmHeight = atoi(argv[++i]);
		}
		else if (!strncmp(argv[i], "-filter", 7))
		{
			options.FilterName = argv[++i];
		}
		else if (!strncmp(argv[i], "-sampler", 8))
		{
			options.SamplerName = argv[++i];
		}
		else if (!strncmp(argv[i], "-accelerator", 12))
		{
			options.AcceleratorName = argv[++i];
		}
		else if (!strncmp(argv[i], "-integrator", 11))
		{
			options.IntegratorName = argv[++i];
		}
		else if (!strncmp(argv[i], "-spp", 4))
		{
			options.samplePerPixel = atoi(argv[++i]);
		}
		else if (!strncmp(argv[i], "-xspp", 4))
		{
			options.xSpp = atoi(argv[++i]);
		}
		else if (!strncmp(argv[i], "-yspp", 4))
		{
			options.ySpp = atoi(argv[++i]);
		}
		else
		{
			filenames.push_back(argv[i]);
		}
	}

	ImageIO::InitPath(rootPath);

	Log::Info("integrator:{}", options.IntegratorName);
	Log::Info("sampler:{}", options.SamplerName);
	Log::Info("xspp:{}", options.xSpp);
	Log::Info("yspp:{}", options.ySpp);
	Log::Info("image size:{},{}", options.filmWidth, options.filmHeight);
	Renderer::GetInstance().Init(options);
	
	Log::Info("ParseScene {}......", filenames[0]);
	Renderer::GetInstance().ParseScene(filenames[0]);

	Log::Info("Begin render.......");
	Renderer::GetInstance().Run();
	Renderer::GetInstance().Cleanup();
	//LOG << "render completed!" << endl;
	Log::Info("Render done!");
	system("pause");
	return 0;
}
