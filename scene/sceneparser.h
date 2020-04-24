#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>

namespace AIR
{
	class Light;
	class Primitive;
	class Transform;
	class Shape;
	struct TriangleMesh;

	class SceneParser
	{
	public:
		SceneParser() : cameraTransform(nullptr)
		{}

		bool Load(const std::string& file, std::vector<std::shared_ptr<Light>>& lights,
			std::vector<std::shared_ptr<Primitive>>& primitives);

		Transform* GetCameraTransform()
		{
			return cameraTransform;
		}
	private:
		Transform* ParseTransform(std::ifstream& fs) const;

		std::shared_ptr<Light> ParseLight(std::ifstream& fs) const;
		void ParsePrimitive(std::ifstream& fs, std::vector<std::shared_ptr<Primitive>>& primitives) const;

		void ParseTriangleMesh(std::ifstream& fs) const;
		Transform* cameraTransform;

		std::vector<std::shared_ptr<TriangleMesh>> triangleMeshes;
	};
}
