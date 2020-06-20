#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include "../RayTracing.h"

namespace AIR
{
	class Light;
	class Primitive;
	class Transform;
	class Shape;
	class Material;
	struct TriangleMesh;
	struct CameraParam;
	class RGBSpectrum;
	class Medium;

	template <typename T>
	class Texture;

	class SceneParser
	{
	public:
		SceneParser() : cameraTransform(nullptr)
		{}

		bool Load(const std::string& file, CameraParam& cameraParam,
			std::vector<std::shared_ptr<Light>>& lights,
			std::vector<std::shared_ptr<Primitive>>& primitives,
			std::vector<std::shared_ptr<Medium>>& mediums);

		Transform* GetCameraTransform()
		{
			return cameraTransform;
		}

		//float GetCameraFOV() const
		//{
		//	return cameraFOV;
		//}

		//bool IsCameraOrtho() const
		//{
		//	return cameraOrtho;
		//}
	private:
		Transform* ParseTransform(std::ifstream& fs) const;

		std::shared_ptr<Medium> ParseMedium(std::ifstream& fs);

		void ParseCamera(CameraParam& cameraParam, std::ifstream& fs);

		std::shared_ptr<Light> ParseLight(std::ifstream& fs, std::vector<std::shared_ptr<Medium>>& mediums) const;
		void ParsePrimitive(std::ifstream& fs, 
			std::vector<std::shared_ptr<Primitive>>& primitives, 
			std::vector<std::shared_ptr<Light>>& lights,
			std::vector<std::shared_ptr<Medium>>& mediums) const;

		std::shared_ptr<Material> ParseMaterial(std::ifstream& fs) const;

		std::shared_ptr<Texture<Float>> ParseFloatTexture(std::ifstream& fs) const;
		std::shared_ptr<Texture<RGBSpectrum>> ParseSpectrumTexture(std::ifstream& fs) const;

		void ParseTriangleMesh(std::ifstream& fs);
		Transform* cameraTransform;
		//float cameraFOV;
		//bool  cameraOrtho;

		std::vector<std::shared_ptr<TriangleMesh>> triangleMeshes;
	};
}
