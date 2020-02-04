#include "sphere.h"
#include "efloat.h"

namespace AIR
{
	std::shared_ptr<Sphere> Sphere::CreateSphere(const GeometryParam& param, Transform* pTransform)
	{
		return std::make_shared<Sphere>(param.param.sphere.radius, param.param.sphere.thetaMin, param.param.sphere.thetaMax, param.param.sphere.phiMax, pTransform);
	}

	
	bool Sphere::Intersect(const Ray &r, Float *tHit, Interaction *isect) const
	{
		Vector3f oErr, dErr;
		Ray ray = mTransform->WorldToObjectRay(r, &oErr, &dErr);

		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);

		EFloat a = dx * dx + dy * dy + dz * dz;
		EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
		EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);

		EFloat t0, t1;

		//解方程
		//pbr-book:http://www.pbr-book.org/3ed-2018/Shapes/Spheres.html
		if (!Quadratic(a, b, c, &t0, &t1))
			return false;

		if (t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0)
			return false;

		EFloat tShapeHit = t0;
		if (tShapeHit.LowerBound() <= 0) {
			tShapeHit = t1;
			if (tShapeHit.UpperBound() > ray.tMax)
				return false;
		}

		//compute the hit point along the ray
		Vector3f pHit = ray((Float)tShapeHit);
		//根据pbrt 3.9.4，做二次intersect，但因为效率低，所以用reproject的方法
		//defined in Section 3.9.4, improves the precision of this value.
		//reproject the point onto the surface 
		pHit *= radius / Vector3f::Distance(pHit, Vector3f::zero);

		if (pHit.x == 0 && pHit.z == 0) 
			pHit.x = 1e-5f * radius;
		Float phi = std::atan2(pHit.z, pHit.x);
		if (phi < 0) 
			phi += 2 * Pi;

		//判断是否在范围内
		if ((yMin > -radius && pHit.y < yMin) ||
			(yMax <  radius && pHit.y > yMax) || phi > phiMax)
		{
			//不在范围内，用t1从新计算交点
			if (tShapeHit == t1) 
				return false;
			if (t1.UpperBound() > ray.tMax) 
				return false;
			tShapeHit = t1;

			pHit = ray((Float)tShapeHit);

			pHit *= radius / Vector3f::Distance(pHit, Vector3f::zero);

			if (pHit.x == 0 && pHit.z == 0) 
				pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.z, pHit.x);
			if (phi < 0) 
				phi += 2 * Pi;

			if ((yMin > -radius && pHit.y < yMin) ||
				(yMax <  radius && pHit.y > yMax) || phi > phiMax)
				return false;

			Float u = phi / phiMax;
			Float theta = std::acos(Clamp(pHit.z / radius, -1, 1));
			Float v = (theta - thetaMin) / (thetaMax - thetaMin);
		}


		//compute dpdu dpdv，p对u的偏导数
		Float u = phi / phiMax;
		Float theta = std::acos(Clamp(pHit.z / radius, -1, 1));
		Float v = (theta - thetaMin) / (thetaMax - thetaMin);

		Float yRadius = std::sqrt(pHit.x * pHit.x + pHit.z * pHit.z);
		Float invYRadius = 1 / yRadius;
		Float cosPhi = pHit.x * invYRadius;
		Float sinPhi = pHit.z * invYRadius;
		//u = φ/φmax
		//dpxdu = ∂px/∂u = ∂(rsinθcosφ)/∂u 
		//               = ∂(rsinθcosφ)/∂(φ/φmax)
		//               = rsinθφmax∂(cosφ)/∂φ
		//               = -rsinθsinφφmax = -zφmax

		//∂py/∂u = ∂(rsinθ)/∂u = 0

		//∂pz/∂u = ∂(rsinθsinφ)/∂u
		//       = ∂(rsinθsinφ)/∂(φ/φmax)
		//       = rsinθφmax∂(sinφ)/∂φ
		//       = rsinθcosφφmax∂
		//       = xφmax
		Vector3f dpdu(-phiMax * pHit.z, 0, phiMax * pHit.x);

		//v = (θ - θmin) / (θmax - θmin);
		//dpxdv = ∂px/∂v = ∂(rsinθcosφ)/∂v 
		//               = ∂(rsinθcosφ)/∂((θ - θmin) / (θmax - θmin))
		//               = (θmax - θmin)rcosφ∂(sinθ)/∂(θ - θmin)
		//               = (θmax - θmin)rcosθcosφ = (θmax - θmin)ycosφ

		//∂py/∂v = ∂(rcosθ)/∂v 
		//               = ∂(rcosθ)/∂((θ - θmin) / (θmax - θmin))
		//               = (θmax - θmin)r∂(cosθ)/∂(θ - θmin)
		//               = -(θmax - θmin)rsinθ

		//∂pz/∂v = ∂(rsinθcosφ)/∂v 
		//               = ∂(rsinθsinφ)/∂((θ - θmin) / (θmax - θmin))
		//               = (θmax - θmin)rsinφ∂(sinθ)/∂(θ - θmin)
		//               = (θmax - θmin)rsinφsinθ = (θmax - θmin)ysinφ
		Vector3f dpdv = (thetaMax - thetaMin) * Vector3f(pHit.y * cosPhi, -radius * std::sin(theta), pHit.y * sinPhi);

		//计算法线的变化
		//∂²px/∂u² = ∂(-zφmax)/∂(φ/φmax) 
		//         = (φmax)²∂(-z)/∂φ
		//         = -(φmax)²∂(rsinθsinφ)/∂φ
		//         = -(φmax)²rsinθcosφ
		//         = -(φmax)²x

		//∂²py/∂u² = ∂(0)/∂(φ/φmax) 
		//         = 0

		//∂²pz/∂u² = ∂(xφmax)/∂(φ/φmax) 
		//         = (φmax)²∂(x)/∂φ
		//         = (φmax)²∂(rsinθcosφ)/∂φ
		//         = -(φmax)²rsinθsinφ
		//         = -(φmax)²z
		Vector3f d2Pduu = -phiMax * phiMax * Vector3f(pHit.x, 0, pHit.z);

		//∂²px/∂u∂v = ∂(-zφmax)/∂((θ - θmin) / (θmax - θmin))
		//          = -(θmax - θmin)∂(zφmax)/∂θ
		//          = -(θmax - θmin)φmax∂(rsinθsinφ)/∂θ
		//          = -(θmax - θmin)φmax(rcosθsinφ)
		//          = -(θmax - θmin)φmaxysinφ

		//∂²py/∂u∂v = ∂(0)/∂((θ - θmin) / (θmax - θmin)) = 0

		//∂²pz/∂u∂v = ∂(xφmax)/∂((θ - θmin) / (θmax - θmin))
		//           = (θmax - θmin)∂(xφmax)/∂θ
		//           = (θmax - θmin)φmax∂(rsinθcosφ)/∂θ
		//           = (θmax - θmin)φmaxrcosθcosφ
		//           = (θmax - θmin)φmaxycosφ
		Vector3f d2Pduv = (thetaMax - thetaMin) * pHit.y * phiMax *
			Vector3f(-sinPhi, 0, cosPhi);

		//∂²px/∂v² = ∂((θmax - θmin)ycosφ)/∂((θ - θmin) / (θmax - θmin)) 
		//         = (θmax - θmin)(θmax - θmin)∂(ycosφ)/∂(θ - θmin)
		//         = (θmax - θmin)²∂(rcosθcosφ)/∂θ
		//         = -(θmax - θmin)²rsinθcosφ
		//         = -(θmax - θmin)²x

		//∂²py/∂v² = ∂(-(θmax - θmin)rsinθ)/∂((θ - θmin) / (θmax - θmin))
		//         = -(θmax - θmin)²∂(rsinθ)/∂(θ)
		//         = -(θmax - θmin)²rcosθ
		//         = -(θmax - θmin)²y
		Vector3f d2Pdvv = -(thetaMax - thetaMin) * (thetaMax - thetaMin) *
			Vector3f(pHit.x, pHit.y, pHit.z);

		Float E = Vector3f::Dot(dpdu, dpdu);
		Float F = Vector3f::Dot(dpdu, dpdv);
		Float G = Vector3f::Dot(dpdv, dpdv);
		Vector3f N = Vector3f::Normalize(Vector3f::Cross(dpdu, dpdv));
		Float e =  Vector3f::Dot(N, d2Pduu);
		Float f =  Vector3f::Dot(N, d2Pduv);
		Float g =  Vector3f::Dot(N, d2Pdvv);

		Float invEGF2 = 1 / (E * G - F * F);
		Vector3f dndu = Vector3f((f * F - e * G) * invEGF2 * dpdu +
			(e * F - f * E) * invEGF2 * dpdv);
		Vector3f dndv = Vector3f((g * F - f * G) * invEGF2 * dpdu +
			(f * F - g * E) * invEGF2 * dpdv);

		Vector3f pError = gamma(5) * Vector3f::Abs(pHit);

		//把interaction转回世界坐标
		*isect = mTransform->ObjectToWorldInteraction(Interaction(pHit, Vector3f::Normalize(Vector3f::Cross(dpdu, dpdv)), Vector2f(u, v), pError, -ray.d, dpdu, dpdv, dndu, dndv, ray.time));
		*tHit = (Float)tShapeHit;
		return true;
	}
	
}
