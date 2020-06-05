﻿#include "disk.h"
#include "sampling.h"

namespace AIR
{
	Bounds3f Disk::ObjectBound() const
	{
		return Bounds3f(Point3f(-radius, -radius, height), Point3f(radius, radius, height));
	}

	Float Disk::Area() const
	{
		//环形：A = πr² - πri²
		//扇形：A = φmax/2(r² - ri²)
		return phiMax * (radius * radius - innerRadius * innerRadius) * 0.5f;
	}

	bool Disk::Intersect(const Ray& r, Float* tHit, SurfaceInteraction* isect) const
	{
		//把ray 转换到object space
		Vector3f oErr, dErr;
		Ray rayObject = mTransform->WorldToObjectRay(r, &oErr, &dErr);

		//如果ray和disk的平面平行，返回false
		if (rayObject.d.z == 0)
			return false;

		//找到hitpoint
		//oz + tdz = height
		//t = (height - oz) / dz
		Float tHitShape = (height - rayObject.d.z) / rayObject.d.z;
		if (tHitShape <= 0 || tHitShape >= rayObject.tMax)
			return false;

		Point3f pHit = rayObject(tHitShape);

		//判断pHit是否在radius内
		Float hitToCenter2 = pHit.x * pHit.x + pHit.y * pHit.y;
		if (hitToCenter2 > radius* radius || hitToCenter2 < innerRadius * innerRadius)
			return false;

		//判断是否在phiMax范围内
		//φMax是斜边与X的夹角
		//只要pHit的xy小于φmax，pHit就是在disk内
		Float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0)   //说明是负数或>180度的
			phi += Pi * 2;
		if (phi > phiMax)
			return false;

		//参数方程：
		//φ = uφmax
		//x = ((1 - v)r + vr_i)cosφ
		//y = ((1 - v)r + vr_i)sinφ
		//z = height
		Float u = phi / phiMax;
		Float hit2Center = std::sqrt(hitToCenter2);
		Float oneMinusV = (hit2Center - innerRadius) / (radius - innerRadius);
		//v的方向是从外到内
		Float v = 1.0f - oneMinusV;

		//参数方程x,y,z分别求u,v偏导
		//∂x/∂u = -φmax y 
		//∂y/∂u = -φmax x 
		//∂z/∂u = 0
		Vector3f dpdu = Vector3f(-phiMax * pHit.y, phiMax * pHit.x, 0);
		Vector3f dpdv = Vector3f(pHit.x, pHit.y, 0.) * (innerRadius - radius) / hit2Center;

		//法线永远向z方向没任何变化
		Vector3f dndu = Vector3f::zero;
		Vector3f dndv = Vector3f::zero;
		Vector3f pError(0, 0, 0);
		*isect = mTransform->ObjectToWorldInteraction(SurfaceInteraction(pHit, pError, Point2f(u, v),
			-rayObject.d, dpdu, dpdv, dndu, dndv,
			rayObject.time, this));

		*tHit = (Float)tHitShape;

		return true;
	}

	bool Disk::IntersectP(const Ray& r) const
	{
		// Transform _Ray_ to object space
		Vector3f oErr, dErr;
		Ray ray = mTransform->WorldToObjectRay(r, &oErr, &dErr);

		// Compute plane intersection for disk

		// Reject disk intersections for rays parallel to the disk's plane
		if (ray.d.z == 0) return false;
		Float tShapeHit = (height - ray.o.z) / ray.d.z;
		if (tShapeHit <= 0 || tShapeHit >= ray.tMax) 
			return false;

		// See if hit point is inside disk radii and $\phimax$
		Point3f pHit = ray(tShapeHit);
		Float dist2 = pHit.x * pHit.x + pHit.y * pHit.y;
		if (dist2 > radius* radius || dist2 < innerRadius * innerRadius)
			return false;

		// Test disk $\phi$ value against $\phimax$
		Float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;
		if (phi > phiMax) 
			return false;
		return true;
	}

	Interaction Disk::Sample(const Point2f& u, Float* pdf) const
	{
		Point2f pd = ConcentricSampleDisk(u);
		Point3f pObj(pd.x * radius, pd.y * radius, height);
		Interaction it;
		it.normal = Vector3f::Normalize(mTransform->ObjectToWorldNormal(Vector3f::forward));
		//if (reverseOrientation) 
		//   it.normal *= -1;
		it.interactPoint = mTransform->ObjectToWorldPoint(pObj, Vector3f(0, 0, 0), &it.pError);
		*pdf = 1 / Area();
		return it;
	}
}
