#include "camera.h"
#include "film.h"
#include "sampling.h"
#include "medium.h"

namespace AIR
{
	Camera* Camera::CreateCamera(const Transform& cameraToWorld, 
		const Bounds2f& screenWindow, 
		Film* film, Float fov, bool orthogonal,
		const Medium* medium)
	{
		return new Camera(cameraToWorld, screenWindow, film, fov, orthogonal, medium);
	}

	Camera::Camera(const Transform& transform, const Bounds2f& screenWindow,
		Film* film, Float fov, bool orthogonal, const Medium* medium) : film(film), orthogonal(orthogonal)
		, medium(medium)
	{
		mTransform = transform;
		Point2i imageResolution = film->fullResolution;
		ScreenToRaster = Matrix4f::GetScaleMatrix(imageResolution.x,
			imageResolution.y, 1) *
			Matrix4f::GetScaleMatrix(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
				1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
			Matrix4f::GetTranslateMatrix(-screenWindow.pMin.x, -screenWindow.pMax.y, 0);
		RasterToScreen = Matrix4f::Inverse(ScreenToRaster);
		
		Float aspect = (Float)imageResolution.x / imageResolution.y;
		CameraToScreen = orthogonal ? Matrix4f::Orthogonal(1e-2f, 1000.f) : 
			Matrix4f::Perspective(fov, aspect, 1e-2f, 1000.f);

		RasterToCamera = Matrix4f::Inverse(CameraToScreen) * RasterToScreen;
		

		dxCamera = (MultiplyPoint(RasterToCamera, Point3f(1, 0, 0)) - MultiplyPoint(RasterToCamera, Point3f(0, 0, 0)));
		dyCamera = (MultiplyPoint(RasterToCamera, Point3f(0, 1, 0)) - MultiplyPoint(RasterToCamera, Point3f(0, 0, 0)));

		if (!orthogonal)
		{
			// Compute image plane bounds at $z=1$ for _PerspectiveCamera_
			Point2i res = imageResolution;
			Point3f pMin = MultiplyPoint(RasterToCamera, Point3f(0, 0, 0));
			Point3f pMax = MultiplyPoint(RasterToCamera, Point3f(res.x, res.y, 0));
			pMin /= pMin.z;
			pMax /= pMax.z;
		}
		
	}

	Float Camera::GenerateRay(const CameraSample &sample, Ray *ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		//pCamera是camera近截面上的点
		Point3f pCamera = MultiplyPoint(RasterToCamera, pFilm);

		*ray = Ray(Point3f(0, 0, 0), Vector3f::Normalize(pCamera));

		ray->time = 0.0f;//Lerp(sample.time, shutterOpen, shutterClose);
		ray->medium = medium;
		*ray = mTransform.ObjectToWorldRay(*ray);

		return 1;
	}

	Float Camera::GenerateRayDifferential(const CameraSample &sample, RayDifferential *ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
	    //pCamera是camera近截面上的点
		Point3f pCamera = MultiplyPoint(RasterToCamera, pFilm);
		Vector3f dir = Vector3f::Normalize(pCamera);
		*ray = RayDifferential(Point3f(0, 0, 0), dir);

		//Float wt = GenerateRay(sample, ray);
		//if (wt == 0)
		//	return 0;
		// Modify ray for depth of field
		if (lensRadius > 0) 
		{
			// Sample point on lens
			Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

			// Compute point on plane of focus
			Float ft = focalDistance / ray->d.z;
			Point3f pFocus = (*ray)(ft);

			// Update ray for effect of lens
			ray->o = Point3f(pLens.x, pLens.y, 0);
			ray->d = Vector3f::Normalize(pFocus - ray->o);
		}

		// Compute offset rays for _PerspectiveCamera_ ray differentials
		if (lensRadius > 0) 
		{
			// Compute _PerspectiveCamera_ ray differentials accounting for lens

			// Sample point on lens
			Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
			Vector3f dx = Vector3f::Normalize(Vector3f(pCamera + dxCamera));
			Float ft = focalDistance / dx.z;
			Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
			ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
			ray->rxDirection = Vector3f::Normalize(pFocus - ray->rxOrigin);

			Vector3f dy = Vector3f::Normalize(Vector3f(pCamera + dyCamera));
			ft = focalDistance / dy.z;
			pFocus = Point3f(0, 0, 0) + (ft * dy);
			ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
			ray->ryDirection = Vector3f::Normalize(pFocus - ray->ryOrigin);
		}
		else {
			ray->rxOrigin = ray->ryOrigin = ray->o;
			ray->rxDirection = Vector3f::Normalize(pCamera + dxCamera);
			ray->ryDirection = Vector3f::Normalize(pCamera + dyCamera);
		}
		ray->time = 0;// Lerp(sample.time, shutterOpen, shutterClose);
		ray->medium = medium;
		*ray = mTransform.ObjectToWorldRayDiff(*ray);

		/*
		// Find camera ray after shifting a fraction of a pixel in the $x$ direction
		Float wtx = 0;
		for (Float eps : { .05, -.05 }) {
			CameraSample sshift = sample;
			sshift.pFilm.x += eps;
			Ray rx;
			wtx = GenerateRay(sshift, &rx);
			ray->rxOrigin = ray->o + (rx.o - ray->o) / eps;
			ray->rxDirection = ray->d + (rx.d - ray->d) / eps;
			if (wtx != 0)
				break;
		}
		if (wtx == 0)
			return 0;

		// Find camera ray after shifting a fraction of a pixel in the $y$ direction
		Float wty = 0;
		for (Float eps : { .05, -.05 }) {
			CameraSample sshift = sample;
			sshift.pFilm.y += eps;
			Ray ry;
			wty = GenerateRay(sshift, &ry);
			ray->ryOrigin = ray->o + (ry.o - ray->o) / eps;
			ray->ryDirection = ray->d + (ry.d - ray->d) / eps;
			if (wty != 0)
				break;
		}
		if (wty == 0)
			return 0;
			*/

		ray->hasDifferentials = true;
		return 1;
	}
}
