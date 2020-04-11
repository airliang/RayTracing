//a perspective camera implementation
#pragma once
#include "robject.h"

namespace AIR
{
	//摄像机的样本，用于生成ray
	struct CameraSample {
		//pFilm 样本在film中的位置（光栅化位置的随机偏移）
		Point2f pFilm;
		Point2f pLens;
		Float time;
	};
	class Film;

	//默认就是一个perspective camera
	class Camera : public RObject
	{
	public:
		Camera(const Transform& transform, const Bounds2f& screenWindow, const Point2i& imageResolution
			, Film* film, bool orthogonal);
		~Camera()
		{

		}

		Float GenerateRay(const CameraSample& sample,
			Ray* ray) const;

		Float GenerateRayDifferential(const CameraSample& sample, RayDifferential* ray) const;

		static std::shared_ptr<Camera> CreateCamera(const Transform& cameraToWorld, const Transform& cameraToScreen);

		const Transform& CameraToWorld() const
		{
			return mTransform;
		}

		Film* film;
	protected:
		Matrix4f CameraToScreen, RasterToCamera;
		Matrix4f ScreenToRaster, RasterToScreen;

		Vector3f dxCamera, dyCamera;
		const bool orthogonal = false;
	};
};
