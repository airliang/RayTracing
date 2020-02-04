#include "camera.h"

namespace AIR
{
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
