#pragma once
#include "geometry.h"
#include "spectrum.h"
#include "medium.h"

namespace AIR
{
	class Primitive;
	class BSDF;
	class Shape;
	class MemoryArena;

	struct Interaction {
		// Interaction Public Methods
		Interaction() : time(0) {}
		Interaction(const Point3f& p, const Vector3f& n, const Vector3f& pError,
			const Vector3f& wo, Float time,
			const MediumInterface& mediumInterface)
			: interactPoint(p),
			time(time),
			pError(pError),
			wo(Vector3f::Normalize(wo)),
			normal(n),
			mediumInterface(mediumInterface) {}
		bool IsSurfaceInteraction() const 
		{ 
			return normal != Vector3f::zero;
		}
		Ray SpawnRay(const Vector3f& d) const 
		{
			Point3f o = OffsetRayOrigin(interactPoint, pError, normal, d);
			return Ray(o, d, Infinity, time, GetMedium(d));
		}
		Ray SpawnRayTo(const Point3f& p2) const 
		{
			Point3f origin = OffsetRayOrigin(interactPoint, pError, normal, p2 - interactPoint);
			Vector3f d = p2 - interactPoint;
			return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
		}

		Ray SpawnRayTo(const Interaction& it) const 
		{
			Point3f origin = OffsetRayOrigin(interactPoint, pError, normal, it.interactPoint - interactPoint);
			Point3f target = OffsetRayOrigin(it.interactPoint, it.pError, it.normal, origin - it.interactPoint);
			Vector3f d = target - origin;
			return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
		}

		Interaction(const Point3f& p, const Vector3f& wo, Float time,
			const MediumInterface& mediumInterface)
			: interactPoint(p), time(time), wo(wo), mediumInterface(mediumInterface) {}
		Interaction(const Point3f& p, Float time,
			const MediumInterface& mediumInterface)
			: interactPoint(p), time(time), mediumInterface(mediumInterface) {}

		bool IsMediumInteraction() const 
		{ 
			return !IsSurfaceInteraction(); 
		}

		const Medium* GetMedium(const Vector3f& w) const {
			return Vector3f::Dot(w, normal) > 0 ? mediumInterface.outside : mediumInterface.inside;
		}
		const Medium* GetMedium() const {
			//CHECK_EQ(mediumInterface.inside, mediumInterface.outside);
			return mediumInterface.inside;
		}

		// Interaction Public Data
		Vector3f interactPoint;   //交点
		Float time;        //应该是相交的ray的参数t
		Vector3f pError;   //floating-point error
		Vector3f wo;
		Vector3f normal;
		MediumInterface mediumInterface;
	};


	class SurfaceInteraction : public Interaction
	{
	public:
		SurfaceInteraction() {}

		SurfaceInteraction(const Point3f& p, const Vector3f& pError,
			const Point2f& uv, const Vector3f& wo,
			const Vector3f& dpdu, const Vector3f& dpdv,
			const Vector3f& dndu, const Vector3f& dndv, Float time,
			const Shape* sh);

		~SurfaceInteraction()
		{

		}

		void SetGeometryShading(const Vector3f &dpdus,
			const Vector3f &dpdvs,
			const Vector3f &dndus,
			const Vector3f &dndvs,
			bool orientationIsAuthoritative);

		void ComputeScatteringFunctions(
			const RayDifferential& ray, MemoryArena& arena,
			bool allowMultipleLobes = false,
			TransportMode mode = TransportMode::Radiance);

		//如果interaction上带有AreaLight
		//the surface can emit the radiance
		//return the emitted radiance in w direction
		Spectrum Le(const Vector3f& w) const;

		//先求出ray和SurfaceInteraction point所在的平面的相交
		//因为这里不知道具体的模型轮廓，所以只能用平面来模拟！
		//假设该点的平面是：ax + by + cz + d = 0
		//其中平面法线是(a,b,c)，d = -(n·p)
		//ray方程是：o + td,代入平面方程得：
		//a(ox + tdx) + b(oy + tdy) + c(oz + tdz) - (n·p) = 0
		// n = (a,b,c)
		//(a,b,c)·o + t(a,b,c)·d = n·p
		//t = (n·p - n·o) / n·d
		void ComputeDifferentials(const RayDifferential& ray) const;	

		Vector2f uv;          //parameterization of surface;
		Vector3f dpdu, dpdv;  //partial derivatives of the point,can see as tangents
		Vector3f dndu, dndv;  //differential change in surface normal

		struct 
		{
			Vector3f n;
			Vector3f dpdu, dpdv;
			Vector3f dndu, dndv;
		} shading;

		//交点p对输出图像的x,y坐标的偏导数
		mutable Vector3f dpdx, dpdy;
		mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

		const Primitive* primitive = nullptr;
		BSDF* bsdf = nullptr;
		const Shape* shape = nullptr;
	};
};


