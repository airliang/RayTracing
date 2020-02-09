//a perspective camera implementation
#pragma once
#include "robject.h"

namespace AIR
{
	struct CameraSample {
		Point2f pFilm;
		Point2f pLens;
		Float time;
	};


	class Camera : public RObject
	{
	public:
		Camera(const Transform& transform, const Bounds2f &screenWindow, const Point2i& imageResolution) : RObject(nullptr)
		{
			mTransform = transform;
			ScreenToRaster = Matrix4f::GetScaleMatrix(imageResolution.x,
				imageResolution.y, 1) *
				Matrix4f::GetScaleMatrix(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
					1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
				Matrix4f::GetTranslateMatrix(-screenWindow.pMin.x, -screenWindow.pMax.y, 0);
			RasterToScreen = Matrix4f::Inverse(ScreenToRaster);
			//����x,y��ͶӰ��z = 1��ƽ���ϣ�������near���ϣ�near��farֻ��Ϊ����z�䵽0-1֮��
			//
			float n = 0.1f;
			float f = 1000.0f;
			CameraToScreen = Matrix4f(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, f / (f - n), -f * n / (f - n),
				0, 0, 1, 0);

			RasterToCamera = Matrix4f::Inverse(CameraToScreen) * RasterToScreen;
			//����ǳ���֣������RasterToCamera�ļ��㣬�����Ľ����x,y��z = 1�����꣬��z��nearplane������
			//�������������nearƽ���ϵ�

			dxCamera = (MultiplyPoint(RasterToCamera, Point3f(1, 0, 0)) - MultiplyPoint(RasterToCamera, Point3f(0, 0, 0)));
			dyCamera = (MultiplyPoint(RasterToCamera, Point3f(0, 1, 0)) - MultiplyPoint(RasterToCamera, Point3f(0, 0, 0)));

			// Compute image plane bounds at $z=1$ for _PerspectiveCamera_
			Point2i res = imageResolution;
			Point3f pMin = MultiplyPoint(RasterToCamera, Point3f(0, 0, 0));
			Point3f pMax = MultiplyPoint(RasterToCamera, Point3f(res.x, res.y, 0));
			pMin /= pMin.z;
			pMax /= pMax.z;
		}
		~Camera()
		{

		}

		Float GenerateRay(const CameraSample &sample,
			Ray *ray) const;

		Float GenerateRayDifferential(const CameraSample &sample, RayDifferential *ray) const;

		static std::shared_ptr<Camera> CreateCamera(const Transform& cameraToWorld, const Transform& cameraToScreen);

		const Transform& CameraToWorld() const
		{
			return mTransform;
		}
	protected:
		Matrix4f CameraToScreen, RasterToCamera;
		Matrix4f ScreenToRaster, RasterToScreen;

		Vector3f dxCamera, dyCamera;
	};
};
