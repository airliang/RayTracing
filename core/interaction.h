#pragma once
#include "geometry.h"

namespace AIR
{
	class RObject;
	class Interaction
	{
	public:
		Interaction() {}
		Interaction(const Vector3f &p, const Vector3f &n, const Vector2f &uv, const Vector3f &error,
			const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
			const Vector3f &dndu, const Vector3f &dndv, Float t) : interactPoint(p), time(t), pError(error)
			, wo(Vector3f::Normalize(wo)), normal(n), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv)
		{

		}

		~Interaction()
		{

		}

		Vector3f interactPoint;   //交点
		Float time;        //应该是相交的ray的参数t
		Vector3f pError;   //floating-point error
		Vector3f wo;
		Vector3f normal;

		Vector2f uv;          //parameterization of surface;
		Vector3f dpdu, dpdv;  //partial derivatives of the point,can see as tangents
		Vector3f dndu, dndv;  //differential change in surface normal
		mutable Vector3f dpdx, dpdy;
		mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

		const RObject* robject = nullptr;
	};
};


