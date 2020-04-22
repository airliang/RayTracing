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
	cout << "machine epsilon=" << MachineEpsilon << endl;

	Transform transform;
	std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(1.0f, 0.0f, Pi, 2.0f * Pi, &transform);
	Ray ray(Vector3f(0, 0, -2.0f), Vector3f::forward);
	Interaction interaction;
	Float tHit;
	sphere->Intersect(ray, &tHit, &interaction);

	Film film(Point2i(512, 512), Bounds2f(Vector2f(0.0f, 0.0f), Vector2f(512.0f, 512.0f)), std::unique_ptr<Filter>(new BoxFilter(Vector2f::one)), "test.exr");
	film.WriteImage();

	RunOptions options;
	std::vector<std::string> filenames;

	for (int i = 1; i < argc; ++i)
	{
		if (!strncmp(argv[i], "--nthreads=", 11)) 
		{
			options.nThreads = atoi(&argv[i][11]);
		}
		else
		{
			filenames.push_back(argv[i]);
		}
	}

	Renderer::GetInstance().Init();
	Renderer::GetInstance().Run();
	Renderer::GetInstance().Cleanup();

	system("pause");
	return 0;
}
