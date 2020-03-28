#pragma once
#include "geometry.h"

namespace AIR
{
	class RObject;
	class BSDF;
	class Shape;
	class Interaction
	{
	public:
		Interaction() {}
		Interaction(const Vector3f &p, const Vector3f &n, const Vector2f &uv, const Vector3f &error,
			const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
			const Vector3f &dndu, const Vector3f &dndv, Float t) : interactPoint(p), time(t), pError(error)
			, wo(Vector3f::Normalize(wo)), normal(n), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv)
		{
			shading.n = n;
			shading.dndu = dndu;
			shading.dndv = dndv;
			shading.dpdu = dndu;
			shading.dpdv = dpdv;
		}

		Interaction(const Point3f& p, Float time)
			: interactPoint(p), time(time) {}

		Interaction(const Point3f& p, const Vector3f& pError,
			const Point2f& uv, const Vector3f& wo,
			const Vector3f& dpdu, const Vector3f& dpdv,
			const Vector3f& dndu, const Vector3f& dndv, Float time,
			const Shape* sh);

		~Interaction()
		{

		}

		void SetGeometryShading(const Vector3f &dpdus,
			const Vector3f &dpdvs,
			const Vector3f &dndus,
			const Vector3f &dndvs,
			bool orientationIsAuthoritative);

		//�����ray��Interaction point���ڵ�ƽ����ཻ
		//��Ϊ���ﲻ֪�������ģ������������ֻ����ƽ����ģ�⣡
		//����õ��ƽ���ǣ�ax + by + cz + d = 0
		//����ƽ�淨����(a,b,c)��d = -(n��p)
		//ray�����ǣ�o + td,����ƽ�淽�̵ã�
		//a(ox + tdx) + b(oy + tdy) + c(oz + tdz) - (n��p) = 0
		// n = (a,b,c)
		//(a,b,c)��o + t(a,b,c)��d = n��p
		//t = (n��p - n��o) / n��d
		void ComputeDifferentials(const RayDifferential& ray) const;

		Vector3f interactPoint;   //����
		Float time;        //Ӧ�����ཻ��ray�Ĳ���t
		Vector3f pError;   //floating-point error
		Vector3f wo;
		Vector3f normal;

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

		const RObject* robject = nullptr;
		BSDF* bsdf = nullptr;
		const Shape* shape = nullptr;
	};
};


