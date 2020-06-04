//a perspective camera implementation
#pragma once
//#include "robject.h"
#include "geometry.h"
#include "transform.h"

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
	class Medium;

	//默认就是一个perspective camera
	class Camera
	{
	public:
		//transform 摄像机的transform
		//screenWindow 近截面的视口大小
		//film      成像的胶片
		//fov       垂直方向的field of view视口角度，只用于perspective
		//orthogonal 是否是正交投影
		Camera(const Transform& transform, const Bounds2f& screenWindow, 
			Film* film, Float fov, bool orthogonal, const Medium* medium);
		~Camera()
		{
			if (film)
				delete film;
		}

		Float GenerateRay(const CameraSample& sample,
			Ray* ray) const;

		Float GenerateRayDifferential(const CameraSample& sample, RayDifferential* ray) const;

		static Camera* CreateCamera(const Transform& cameraToWorld, const Bounds2f& screenWindow,
			Film* film, Float fov, bool orthogonal, const Medium* medium);

		const Transform& CameraToWorld() const
		{
			return mTransform;
		}

		Film* film;
	protected:
		//cameraToScreen就是投影矩阵,投影到x=[-1,1]
		//y=[-1,1],z=[0,1]的box上
		Matrix4f CameraToScreen, RasterToCamera;
		Matrix4f ScreenToRaster, RasterToScreen;

		Vector3f dxCamera, dyCamera;
		const bool orthogonal = false;

		Transform mTransform;
		Float lensRadius = 0;
		Float focalDistance = 0;
		//摄像机所处环境的medium，
		//在camera生成的ray需要携带这个medium
		const Medium* medium;
	};
};
