#include "sceneparser.h"
#include "transform.h"
#include "geometry.h"
#include "quaternion.h"
#include "renderer.h"
#include "distantlight.h"
#include "pointlight.h"
#include "diffusearealight.h"
#include "disk.h"
#include "sphere.h"
#include "triangle.h"
#include "plasticmaterial.h"
#include "matte.h"
#include "mirrormaterial.h"
#include "constanttexture.h"
#include "../RayTracing.h"

namespace AIR
{
	enum LightType
	{
		Spot = 0,

		//     The light is a directional light.
		Directional = 1,

		//     The light is a point light.
		Point = 2,
		Area = 3,

		//     The light is a rectangle shaped area light. It affects only baked lightmaps and
		//     lightprobes.
		Rectangle = 3,

		//     The light is a disc shaped area light. It affects only baked lightmaps and lightprobes.
		Disc = 4
	};

	enum ShapeType
	{
		ShapeType_Sphere,
		ShapeType_TriangleMesh,
		ShapeType_Rectangle,
	};

	enum MaterialType
	{
		MaterialType_Matte,
		MaterialType_Plastic,
		MaterialType_Mirror,
		MaterialType_Metal,
	};

	enum TextureType
	{
		TextureType_Constant,
		TextureType_Bilerp,
		TextureType_Image,
	};

	bool SceneParser::Load(const std::string& file, std::vector<std::shared_ptr<Light>>& lights,
		std::vector<std::shared_ptr<Primitive>>& primitives)
	{
		size_t ex = file.find_last_of('.');
		if (ex >= 0)
		{
			std::string meshFile = file.substr(0, ex) + ".mesh";
			std::ifstream fsmesh;
			fsmesh.open(meshFile.c_str(), std::ios::in | std::ios::binary);
			ParseTriangleMesh(fsmesh);
			fsmesh.close();
		}

		std::ifstream fs;

		fs.open(file.c_str(), std::ios::in | std::ios::binary);

		//read the camera's transform
		ParseCamera(fs);

		int lightsNum = 0;
		fs.read((char*)&lightsNum, sizeof(lightsNum));
		for (int i = 0; i < lightsNum; ++i)
		{
			lights.push_back(ParseLight(fs));
		}

		int shapesNum = 0;
		fs.read((char*)&shapesNum, sizeof(shapesNum));
		for (int i = 0; i < shapesNum; ++i)
		{
			ParsePrimitive(fs, primitives);
		}



		return false;
	}

	void SceneParser::ParseCamera(std::ifstream& fs)
	{
		cameraTransform = ParseTransform(fs);
		fs.read((char*)&cameraFOV, sizeof(cameraFOV));
		fs.read((char*)&cameraOrtho, sizeof(cameraOrtho));
	}

	Transform* SceneParser::ParseTransform(std::ifstream& fs) const
	{
		Vector3f pos;
		fs.read((char*)&pos, sizeof(Vector3f));
		Quaternion rotation;
		fs.read((char*)&rotation, sizeof(Quaternion));
		bool ignoreScale = true;
		fs.read((char*)&ignoreScale, sizeof(ignoreScale));
		Vector3f scale = Vector3f::one;
		if (!ignoreScale)
		{
			fs.read((char*)&scale, sizeof(Vector3f));
		}

		Transform transform = Transform::MakeTransform(pos, rotation, scale);
		return TransformCache::GetInstance().Lookup(transform);
	}

	std::shared_ptr<Light> SceneParser::ParseLight(std::ifstream& fs) const
	{
		std::shared_ptr<Light> light;
		Transform* pTransform = ParseTransform(fs);
		int lightType;
		fs.read((char*)&lightType, sizeof(lightType));
		RGBSpectrum color;
		fs.read((char*)&color, sizeof(RGBSpectrum));
		float intensity = 1;
		fs.read((char*)&intensity, sizeof(intensity));
		Spectrum I(intensity);

		if (lightType == LightType::Directional)
		{
			Vector3f dir;
			fs.read((char*)&dir, sizeof(Vector3f));
			light = std::make_shared<DistantLight>(*pTransform, color * I, dir);
		}
		else if (lightType == LightType::Point)
		{
			light = std::make_shared<PointLight>(*pTransform, color * I);
		}
		else if (lightType == LightType::Disc)
		{
			float radius = 0;
			fs.read((char*)&radius, sizeof(radius));
			std::shared_ptr<Disk> disk = std::make_shared<Disk>(pTransform, 0, radius, 0, 2.0f * Pi);
			light = std::make_shared<DiffuseAreaLight>(*pTransform, color * I, 1, disk);
		}
		else if (lightType == LightType::Rectangle)
		{
		}

		return light;
	}

	std::shared_ptr<Material> SceneParser::ParseMaterial(std::ifstream& fs) const
	{
		std::shared_ptr<Material> material;
		int materialType;
		fs.read((char*)&materialType, sizeof(materialType));

		

		if (materialType == MaterialType::MaterialType_Matte)
		{
			std::shared_ptr <Texture< RGBSpectrum >> textureKd = ParseSpectrumTexture(fs);
			std::shared_ptr <Texture< Float >> textureSigma = ParseFloatTexture(fs);

			material = std::make_shared<MatteMaterial>(textureKd, textureSigma, nullptr);
		}
		else if (materialType == MaterialType::MaterialType_Plastic)
		{
			std::shared_ptr <Texture< RGBSpectrum >> textureKd = ParseSpectrumTexture(fs);
			std::shared_ptr <Texture< RGBSpectrum >> textureKs = ParseSpectrumTexture(fs);
			std::shared_ptr <Texture< Float >> textureRoughness = ParseFloatTexture(fs);

			material = std::make_shared<PlasticMaterial>(textureKd, textureKs, textureRoughness, nullptr, false);
		}
		else if (materialType == MaterialType_Mirror)
		{
			std::shared_ptr <Texture< RGBSpectrum >> textureKr = ParseSpectrumTexture(fs);
			material = std::make_shared<MirrorMaterial>(textureKr, nullptr);
		}

		return material;
	}

	std::shared_ptr<Texture<Float>> SceneParser::ParseFloatTexture(std::ifstream& fs) const
	{
		int type;
		
		std::shared_ptr<Texture<Float>> texture;
		fs.read((char*)&type, sizeof(type));
		if (type == TextureType_Constant)
		{
			float constant;
			fs.read((char*)&constant, sizeof(float));
			texture = std::make_shared<ConstantTexture<float>>(constant);
		}
		else if (type == TextureType_Image)
		{
			//暂不支持
		}

		return texture;
	}

	std::shared_ptr<Texture<RGBSpectrum>> SceneParser::ParseSpectrumTexture(std::ifstream& fs) const
	{
		int type;
		
		std::shared_ptr<Texture<RGBSpectrum>> texture;
		fs.read((char*)&type, sizeof(type));
		if (type == TextureType_Constant)
		{
			RGBSpectrum constant;
			fs.read((char*)&constant, sizeof(RGBSpectrum));
			texture = std::make_shared<ConstantTexture<RGBSpectrum>>(constant);
		}
		else if (type == TextureType_Image)
		{
			//暂不支持
		}

		return texture;
	}

	void SceneParser::ParsePrimitive(std::ifstream& fs, std::vector<std::shared_ptr<Primitive>>& primitives) const
	{

		int shapeType;
		fs.read((char*)&shapeType, sizeof(shapeType));

		Transform* pTransform = ParseTransform(fs);

		if (shapeType == ShapeType::ShapeType_Sphere)
		{
			float radius;
			fs.read((char*)&radius, sizeof(radius));
			std::shared_ptr<Shape> shape = std::make_shared<Sphere>(radius, 0, Pi, 2.0f * Pi, pTransform);
			std::shared_ptr<Primitive> primitive = std::make_shared<Primitive>(shape, ParseMaterial(fs), nullptr, pTransform);
			primitives.push_back(primitive);
		}
		else if (shapeType == ShapeType::ShapeType_TriangleMesh)
		{
			int meshIndex;
			fs.read((char*)&meshIndex, sizeof(meshIndex));

			std::shared_ptr<Material> material = ParseMaterial(fs);

			std::shared_ptr<TriangleMesh> mesh = triangleMeshes[meshIndex];

  			for (int i = 0; i < mesh->nTriangles; ++i)
			{
				std::shared_ptr<Shape> shape = std::make_shared<Triangle>(pTransform, mesh, i);
				std::shared_ptr<Primitive> primitive = std::make_shared<Primitive>(shape, material, nullptr, pTransform);
			}
		}

		
	}

	void SceneParser::ParseTriangleMesh(std::ifstream& fs) const
	{
		int meshesNum;
		fs.read((char*)&meshesNum, sizeof(int));
		
		for (int i = 0; i < meshesNum; ++i)
		{
			int vertexCount = 0;
			fs.read((char*)&vertexCount, sizeof(int));

			Point3f* vertices = new Point3f[vertexCount];
			fs.read((char*)vertices, sizeof(Point3f) * vertexCount);
			std::unique_ptr<Point3f[]> p(vertices);

			Vector3f* normals = new Vector3f[vertexCount];
			fs.read((char*)normals, sizeof(Vector3f) * vertexCount);
			std::unique_ptr<Point3f[]> n(normals);

			Vector3f* tangents = new Vector3f[vertexCount];
			fs.read((char*)tangents, sizeof(Vector3f) * vertexCount);
			std::unique_ptr<Point3f[]> t(tangents);

			bool hasuv;
			fs.read((char*)&hasuv, sizeof(bool));
			Vector2f* uv = nullptr;
			if (hasuv)
			{
				uv = new Vector2f[vertexCount];
				fs.read((char*)uv, sizeof(Vector2f) * vertexCount);
			}

			int trianglesNum;
			fs.read((char*)&trianglesNum, sizeof(int));

			int* triangles = new int[trianglesNum];
			fs.read((char*)triangles, sizeof(int) * trianglesNum);
			std::unique_ptr<int[]> tri(triangles);

			std::shared_ptr<TriangleMesh> triangleMesh = 
				std::make_shared<TriangleMesh>(trianglesNum / 3, triangles, vertexCount,
					vertices, tangents, normals, uv);
			
		}
	}
}
