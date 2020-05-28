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
#include "glassmaterial.h"
#include "../RayTracing.h"
#include "imageio.h"
#include "imagetexture.h"

namespace AIR
{
	enum LightType
	{
		delta_distant,
		delta_point,
		area,
	};

	enum ShapeType
	{
		ShapeType_Sphere,
		ShapeType_Disk,
		ShapeType_TriangleMesh,
		ShapeType_Rectangle,
	};

	enum MaterialType
	{
		MaterialType_Matte,
		MaterialType_Plastic,
		MaterialType_Mirror,
		MaterialType_Metal,
		MaterialType_Glass,
	};

	enum TextureType
	{
		TextureType_Constant,
		TextureType_Bilerp,
		TextureType_Image,
	};

	enum ImageWrapMode
	{
		Wrap_Repeat = 0,
		//
		// 摘要:
		//     Clamps the texture to the last pixel at the edge.
		Wrap_Clamp = 1,
		//
		// 摘要:
		//     Tiles the texture, creating a repeating pattern by mirroring it at every integer
		//     boundary.
		Wrap_Mirror = 2,
		//
		// 摘要:
		//     Mirrors the texture once, then clamps to edge pixels.
		Wrap_MirrorOnce = 3
	};

	enum ImageMapping
	{
		ImageMapping_UVMapping2D,
		ImageMapping_Spherical,
	};

	bool SceneParser::Load(const std::string& file, std::vector<std::shared_ptr<Light>>& lights,
		std::vector<std::shared_ptr<Primitive>>& primitives)
	{
		size_t ex = file.find_last_of('.');
		if (ex >= 0)
		{
			std::string meshFile = file.substr(0, ex) + ".m";
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
			std::shared_ptr<Light> light = ParseLight(fs);
			if (light != NULL)
				lights.push_back(light);
		}

		int shapesNum = 0;
		fs.read((char*)&shapesNum, sizeof(shapesNum));
		for (int i = 0; i < shapesNum; ++i)
		{
			ParsePrimitive(fs, primitives, lights);
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
		const Matrix4x4& local2world = transform.LocalToWorld();
		return TransformCache::GetInstance().Lookup(transform);
	}

	std::shared_ptr<Light> SceneParser::ParseLight(std::ifstream& fs) const
	{
		std::shared_ptr<Light> light = nullptr;
		Transform* pTransform = ParseTransform(fs);
		int lightType;
		fs.read((char*)&lightType, sizeof(lightType));
		RGBSpectrum color;
		fs.read((char*)&color, sizeof(RGBSpectrum));
		float intensity = 1;
		fs.read((char*)&intensity, sizeof(intensity));
		Spectrum I(intensity);

		if (lightType == LightType::delta_distant)
		{
			Vector3f dir;
			fs.read((char*)&dir, sizeof(Vector3f));
			light = std::make_shared<DistantLight>(*pTransform, color * I, dir);
		}
		else if (lightType == LightType::delta_point)
		{
			light = std::make_shared<PointLight>(*pTransform, color * I);
		}
		else if (lightType == LightType::area)
		{
			float radius = 0;
			fs.read((char*)&radius, sizeof(radius));
			std::shared_ptr<Disk> disk = std::make_shared<Disk>(pTransform, 0, radius, 0, 2.0f * Pi);
			light = std::make_shared<DiffuseAreaLight>(*pTransform, color * I, 1, disk);
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
		else if (materialType == MaterialType_Metal)
		{
		}
		else if (materialType == MaterialType_Glass)
		{
			std::shared_ptr <Texture< RGBSpectrum >> textureKr = ParseSpectrumTexture(fs);
			std::shared_ptr <Texture< RGBSpectrum >> textureKs = ParseSpectrumTexture(fs);
			std::shared_ptr <Texture< Float >> uRougness = ParseFloatTexture(fs);
			std::shared_ptr <Texture< Float >> vRougness = ParseFloatTexture(fs);
			std::shared_ptr <Texture< Float >> index = ParseFloatTexture(fs);
			material = std::make_shared<GlassMaterial>(textureKr, textureKs, uRougness, vRougness, index, nullptr, true);
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
			char szFilename[256] = { 0 };
			fs.read(szFilename, 256);
			bool doTri = true;

			bool gamma = false;
			fs.read((char*)&gamma, sizeof(gamma));

			int wrap = (int)ImageWrapMode::Wrap_Repeat;
			fs.read((char*)&wrap, sizeof(wrap));
			ImageWrap imWrap = ImageWrap::Repeat;
			if (wrap == (int)ImageWrapMode::Wrap_Clamp)
			{
				imWrap = ImageWrap::Clamp;
			}

			int mapping = (int)ImageMapping::ImageMapping_UVMapping2D;
			fs.read((char*)&mapping, sizeof(mapping));
			std::unique_ptr<TextureMapping2D> m(new UVMapping2D());
			if (mapping == (int)ImageMapping::ImageMapping_UVMapping2D)
			{
				float su, sv, du, dv;
				fs.read((char*)&su, sizeof(su));
				fs.read((char*)&sv, sizeof(sv));
				fs.read((char*)&du, sizeof(du));
				fs.read((char*)&dv, sizeof(dv));
				m.reset(new UVMapping2D(su, sv, du, dv));
			}
			else if (mapping == (int)ImageMapping_Spherical)
			{
				m.reset(new SphericalMapping2D(Matrix4f::IDENTITY));
			}

			texture = std::make_shared<ImageTexture<Float, Float>>(std::move(m), szFilename, doTri, 8.0f, imWrap, 1.0f, gamma);
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
			char szFilename[256] = { 0 };
			fs.read(szFilename, 256);
			//暂不支持
			//Point2i resolution;
			//std::unique_ptr<RGBSpectrum[]> rgbImage = ImageIO::ReadImage(szFilename, resolution);
			bool doTri = true;

			bool gamma = false;
			fs.read((char*)&gamma, sizeof(gamma));

			int wrap = (int)ImageWrapMode::Wrap_Repeat;
			fs.read((char*)&wrap, sizeof(wrap));
			ImageWrap imWrap = ImageWrap::Repeat;
			if (wrap == (int)ImageWrapMode::Wrap_Clamp)
			{
				imWrap = ImageWrap::Clamp;
			}

			int mapping = (int)ImageMapping::ImageMapping_UVMapping2D;
			fs.read((char*)&mapping, sizeof(mapping));
			std::unique_ptr<TextureMapping2D> m(new UVMapping2D());
			if (mapping == (int)ImageMapping::ImageMapping_UVMapping2D)
			{
				float su, sv, du, dv;
				fs.read((char*)&su, sizeof(su));
				fs.read((char*)&sv, sizeof(sv));
				fs.read((char*)&du, sizeof(du));
				fs.read((char*)&dv, sizeof(dv));
				m.reset(new UVMapping2D(su, sv, du, dv));
			}
			else if (mapping == (int)ImageMapping_Spherical)
			{
				m.reset(new SphericalMapping2D(Matrix4f::IDENTITY));
			}

			texture = std::make_shared<ImageTexture<RGBSpectrum, Spectrum>>(std::move(m), szFilename, doTri, 8.0f, imWrap, 1.0f, gamma);
		}

		return texture;
	}

	void SceneParser::ParsePrimitive(std::ifstream& fs, std::vector<std::shared_ptr<Primitive>>& primitives
		, std::vector<std::shared_ptr<Light>>& lights) const
	{

		int shapeType;
		fs.read((char*)&shapeType, sizeof(shapeType));

		Transform* pTransform = ParseTransform(fs);

		bool isAreaLight = false;
		fs.read((char*)&isAreaLight, sizeof(isAreaLight));
		RGBSpectrum lightSpectrum;
		float I = 1.0f;
		if (isAreaLight)
		{
			fs.read((char*)&lightSpectrum, sizeof(lightSpectrum));
			fs.read((char*)&I, sizeof(float));
		}

		if (shapeType == ShapeType::ShapeType_Sphere)
		{
			float radius;
			fs.read((char*)&radius, sizeof(radius));
			std::shared_ptr<Shape> shape = std::make_shared<Sphere>(radius, -radius, radius, 2.0f * Pi, pTransform);
			std::shared_ptr<Primitive> primitive = std::make_shared<Primitive>(shape, ParseMaterial(fs), nullptr, pTransform);
			primitives.push_back(primitive);

			if (isAreaLight)
			{
				std::shared_ptr<Light> light = std::make_shared<DiffuseAreaLight>(*pTransform, lightSpectrum * I, 1, shape);
				lights.push_back(light);
			}
		}
		else if (shapeType == ShapeType::ShapeType_TriangleMesh || shapeType == ShapeType::ShapeType_Rectangle)
		{
			int meshIndex;
			fs.read((char*)&meshIndex, sizeof(meshIndex));

			std::shared_ptr<Material> material = ParseMaterial(fs);

			std::shared_ptr<TriangleMesh> mesh = triangleMeshes[meshIndex];

  			for (int i = 0; i < mesh->nTriangles; ++i)
			{
				std::shared_ptr<Light> light = nullptr;
				std::shared_ptr<Shape> shape = std::make_shared<Triangle>(pTransform, mesh, i);
				if (isAreaLight)
				{
					light = std::make_shared<DiffuseAreaLight>(*pTransform, lightSpectrum * I, 1, shape);
					lights.push_back(light);
				}
				
				std::shared_ptr<Primitive> primitive = std::make_shared<Primitive>(shape, material, std::dynamic_pointer_cast<AreaLight>(light), pTransform);
				primitives.push_back(primitive);
				
			}
		}

		
	}

	void SceneParser::ParseTriangleMesh(std::ifstream& fs)
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
			
			triangleMeshes.push_back(triangleMesh);
		}
	}
}
