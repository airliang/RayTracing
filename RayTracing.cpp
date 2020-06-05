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
#include <filesystem>
using namespace std;
using namespace AIR;
int main(int argc, char* argv[])
{

	cout << "Hello CMake。" << endl;
	auto forward = Vector3f::forward;
	forward = forward.Normalize();
	cout << "sizeof(matrix4x4)=" << sizeof(Matrix4x4) <<endl;
	cout << "sizeof(quaternion)=" << sizeof(Quaternion) <<endl;
	cout << "sizeof(matrix3x3)=" << sizeof(Matrix3x3) << endl;
	cout << "sizeof(Vector3f)=" << sizeof(Vector3f) << endl;
	cout << "sizeof(Vector2f)=" << sizeof(Vector2f) << endl;
	cout << "sizeof(Sphere)=" << sizeof(Sphere) << endl;
	cout << "sizeof(GeometryParam)=" << sizeof(GeometryParam) << endl;
	cout << "sizeof(Transform)=" << sizeof(Transform) << endl;
	cout << "sizeof(RGBSpectrum)=" << sizeof(RGBSpectrum) << endl;
	cout << "machine epsilon=" << MachineEpsilon << endl;

	std::string directory = DirectoryContaining("scene.rt");

	//这里获取全路径
	std::filesystem::path path = std::filesystem::current_path();
	std::string rootPath = path.string();
	int index = rootPath.find("bin");
	rootPath = rootPath.substr(0, index);

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
		else
		{
			filenames.push_back(argv[i]);
		}
	}
	ImageIO::InitPath(rootPath);

	Renderer::GetInstance().Init(options);
	Renderer::GetInstance().ParseScene(filenames[0]);
	Renderer::GetInstance().Run();
	Renderer::GetInstance().Cleanup();
	LOG << "render completed!" << endl;
	system("pause");
	return 0;
}
