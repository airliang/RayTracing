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
	class RGBSpectrum;

	template <typename T>
	class Texture;

	class SceneParser
	{
	public:
		SceneParser() : cameraTransform(nullptr), cameraFOV(0), cameraOrtho(false)
		{}

		bool Load(const std::string& file, std::vector<std::shared_ptr<Light>>& lights,
			std::vector<std::shared_ptr<Primitive>>& primitives);

		Transform* GetCameraTransform()
		{
			return cameraTransform;
		}

		float GetCameraFOV() const
		{
			return cameraFOV;
		}

		bool IsCameraOrtho() const
		{
			return cameraOrtho;
		}
	private:
		Transform* ParseTransform(std::ifstream& fs) const;

		void ParseCamera(std::ifstream& fs);

		std::shared_ptr<Light> ParseLight(std::ifstream& fs) const;
		void ParsePrimitive(std::ifstream& fs, std::vector<std::shared_ptr<Primitive>>& primitives) const;

		std::shared_ptr<Material> ParseMaterial(std::ifstream& fs) const;

		std::shared_ptr<Texture<Float>> ParseFloatTexture(std::ifstream& fs) const;
		std::shared_ptr<Texture<RGBSpectrum>> ParseSpectrumTexture(std::ifstream& fs) const;

		void ParseTriangleMesh(std::ifstream& fs);
		Transform* cameraTransform;
		float cameraFOV;
		bool  cameraOrtho;

		std::vector<std::shared_ptr<TriangleMesh>> triangleMeshes;
	};
}
