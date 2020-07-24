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
	class BSSRDF;

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
		Vector3f interactPoint;   //����
		Float time;        //Ӧ�����ཻ��ray�Ĳ���t
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

		//���interaction�ϴ���AreaLight
		//the surface can emit the radiance
		//return the emitted radiance in w direction
		Spectrum Le(const Vector3f& w) const;

		//�����ray��SurfaceInteraction point���ڵ�ƽ����ཻ
		//��Ϊ���ﲻ֪�������ģ������������ֻ����ƽ����ģ�⣡
		//����õ��ƽ���ǣ�ax + by + cz + d = 0
		//����ƽ�淨����(a,b,c)��d = -(n��p)
		//ray�����ǣ�o + td,����ƽ�淽�̵ã�
		//a(ox + tdx) + b(oy + tdy) + c(oz + tdz) - (n��p) = 0
		// n = (a,b,c)
		//(a,b,c)��o + t(a,b,c)��d = n��p
		//t = (n��p - n��o) / n��d
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

		//����p�����ͼ���x,y�����ƫ����
		mutable Vector3f dpdx, dpdy;
		mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

		const Primitive* primitive = nullptr;
		BSDF* bsdf = nullptr;
		BSSRDF* bssrdf = nullptr;
		const Shape* shape = nullptr;
	};

	class MediumInteraction : public Interaction
	{
	public:
		// MediumInteraction Public Methods
		MediumInteraction() : phase(nullptr) {}
		MediumInteraction(const Point3f& p, const Vector3f& wo, Float time,
			const Medium* medium, const PhaseFunction* phase)
			: Interaction(p, wo, time, medium), phase(phase) {}
		bool IsValid() const 
		{ 
			return phase != nullptr; 
		}

		// MediumInteraction Public Data
		const PhaseFunction* phase;
	};
};


