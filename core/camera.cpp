#include "camera.h"
#include "film.h"

namespace AIR
{
	Camera* Camera::CreateCamera(const Transform& cameraToWorld, 
		const Bounds2f& screenWindow, 
		Film* film, bool orthogonal)
	{
		return new Camera(cameraToWorld, screenWindow, film, orthogonal);
	}

	Camera::Camera(const Transform& transform, const Bounds2f& screenWindow,
		Film* film, bool orthogonal) : film(film), orthogonal(orthogonal)
	{
		mTransform = transform;
		Point2i imageResolution = film->fullResolution;
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

	Float Camera::GenerateRay(const CameraSample &sample, Ray *ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = MultiplyPoint(RasterToCamera, pFilm);

		*ray = Ray(Point3f(0, 0, 0), Vector3f::Normalize(pCamera));

		ray->time = 0.0f;//Lerp(sample.time, shutterOpen, shutterClose);
		//ray->medium = medium;
		*ray = mTransform.ObjectToWorldRay(*ray);

		return 1;
	}

	Float Camera::GenerateRayDifferential(const CameraSample &sample, RayDifferential *ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = MultiplyPoint(RasterToCamera, pFilm);
		Vector3f dir = Vector3f::Normalize(pCamera);
		*ray = RayDifferential(Point3f(0, 0, 0), dir);
		// Modify ray for depth of field
		//if (lensRadius > 0) {
		//	// Sample point on lens
		//	Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

		//	// Compute point on plane of focus
		//	Float ft = focalDistance / ray->d.z;
		//	Point3f pFocus = (*ray)(ft);

		//	// Update ray for effect of lens
		//	ray->o = Point3f(pLens.x, pLens.y, 0);
		//	ray->d = Normalize(pFocus - ray->o);
		//}

		// Compute offset rays for _PerspectiveCamera_ ray differentials
		//if (lensRadius > 0) {
		//	// Compute _PerspectiveCamera_ ray differentials accounting for lens

		//	// Sample point on lens
		//	Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
		//	Vector3f dx = Normalize(Vector3f(pCamera + dxCamera));
		//	Float ft = focalDistance / dx.z;
		//	Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
		//	ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
		//	ray->rxDirection = Normalize(pFocus - ray->rxOrigin);

		//	Vector3f dy = Normalize(Vector3f(pCamera + dyCamera));
		//	ft = focalDistance / dy.z;
		//	pFocus = Point3f(0, 0, 0) + (ft * dy);
		//	ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
		//	ray->ryDirection = Normalize(pFocus - ray->ryOrigin);
		//}
		//else {
			ray->rxOrigin = ray->ryOrigin = ray->o;
			ray->rxDirection = Vector3f::Normalize(pCamera + dxCamera);
			ray->ryDirection = Vector3f::Normalize(pCamera + dyCamera);
		//}
		ray->time = 0;// Lerp(sample.time, shutterOpen, shutterClose);
		//ray->medium = medium;
		*ray = mTransform.ObjectToWorldRay(*ray);
		ray->hasDifferentials = true;
		return 1;
	}
}
