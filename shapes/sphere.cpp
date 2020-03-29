#include "sphere.h"
#include "efloat.h"
#include "sampling.h"

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

	Interaction Sphere::Sample(const Point2f& u, Float* pdf) const
	{
		//采样object space中的一点
		Point3f pObj = Point3f::zero + radius * UniformSampleSphere(u);
		//
		Interaction it;
		//reprojected pobj

		it.normal = Vector3f::Normalize(mTransform->ObjectToWorldNormal(pObj));
		//repojected
		pObj *= radius / Vector3f::Distance(pObj, Point3f::zero);
		Vector3f pObjError = gamma(5) * Vector3f::Abs((Vector3f)pObj);
        it.interactPoint = mTransform->ObjectToWorldPoint(pObj, pObjError, &it.pError);
		return it;
	}
	
	Interaction Sphere::Sample(const Interaction& ref, const Point2f& u,
			Float* pdf) const
	{
		
		Point3f pCenter = mTransform->ObjectToWorldPoint(Point3f::zero);
        

		//Sample uniformly on sphere if p is inside it
		Point3f pOrig = OffsetRayOrigin(ref.interactPoint, ref.pError, ref.normal,
		    pCenter - ref.interactPoint);

        //check is the orig in the sphere 
		if (Vector3f::DistanceSquare(pOrig, pCenter) <= radius * radius)
		{
			Interaction intr = Sample(u, pdf);
			//这里的pdf = 1/A
			Vector3f wi = intr.interactPoint - ref.interactPoint;
			if (wi.LengthSquared() == 0)
				*pdf = 0;
			else 
			{
				// Convert from area measure returned by Sample() call above to
				// solid angle measure.
				wi = Vector3f::Normalize(wi);
				//http://www.pbr-book.org/3ed-2018/Color_and_Radiometry/Working_with_Radiometric_Integrals.html#eq:dw-dA
				//如果dA和ω方向垂直，那么dA和dω的关系是：dA = r²dω。
				//A的法线和ω夹角是θ的时候：dAcosθ = r²dω。
				//1 = ∫p(A)dA = ∫p(A)dA = ∫p(A)r²/cosθdω
				//由于∫p(ω)dω = 1
				//p(A)r²/cosθ = p(ω)，p(A) = 1/A
				//得到：p(ω) = r²/(Acosθ)
				*pdf *= Vector3f::DistanceSquare(ref.interactPoint, intr.interactPoint) 
				    / Vector3f::AbsDot(intr.normal, -wi);
			}
			if (std::isinf(*pdf)) 
			    *pdf = 0.f;

			return intr;
		}

		//转到世界坐标
		Vector3f wc = Vector3f::Normalize(pCenter - ref.interactPoint);
		Vector3f wcX, wcY;
		CoordinateSystem(wc, &wcX, &wcY);

		//sample the cone
		//dω = sinθdθdφ
		//∫p(ω)dω = 1
		//∫∫p(θ, φ)sinθdθdφ = 1
		//p(θ, φ) = p(φ|θ)p(θ)
		//由于在单位半球里，p(ω) = 1/2π，半球面积是2π。
		//根据概率密度函数的转换，单位半球的概率密度
		//p(θ, φ) = sinθp(ω) = sinθ/2π
		//根据边际概率密度：
		//p(θ) = ∫[0, 2π]p(θ, φ)dφ = sinθ
		//由于在cone是hemisphere的一部分，p(θ)=csinθ，然后求出c
		//1 = ∫[0, θmax]csinθdθ
		//= c(1 - cosθmax)
		//∴ c = 1/(1 - cosθmax)
		//p(θ) = sinθ / (1 - cosθmax)
		//1 = ∫[0,2π]∫[0, θmax]p(θ, φ)sinθdθdφ
		//= c/2π∫[0,2π]∫[0, θmax]sinθsinθdθdφ
		//cone在球上的面积是：
		//单位球条带面积：2πsinθdθ
		//cone的面积：∫[0,θmax]2πsinθdθ = 2π(1 - cosθmax)
		//所以cone上的p(ω) = 1 / (2π(1 - cosθmax))
		//p(φ) = 1/2π
        //首先求出cosθmax
		Float sinThetaMax2 = radius * radius / Vector3f::DistanceSquare(ref.interactPoint, pCenter);
		Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
		Float cosTheta = (1 - u[0]) + u[0] * cosThetaMax;
		Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
		Float phi = u[1] * 2 * Pi;

		//http://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Light_Sources.html
		Float dc = Vector3f::Distance(ref.interactPoint, pCenter);
		//ds是ref的样本向量和球面的交点
		//ds = dccosθ - sqrt(r² - dc²sin²θ)
		Float ds = dc * cosTheta - 
		std::sqrt(std::max((Float)0, radius * radius - dc * dc * sinTheta * sinTheta));

		//计算alpha夹角，是球心到球面交点的向量和球心到ref的向量的夹角
		//余弦定理
		Float cosAlpha = (dc * dc + radius * radius - ds * ds) /
                 (2 * dc * radius);

		Float sinAlpha = std::sqrt(std::max((Float)0, 1 - cosAlpha * cosAlpha));

		//最后求出交点p，当前的坐标系是以wc为向上方向的坐标系
		Float phi = u[1] * 2 * Pi;

        // Compute surface normal and sampled point on sphere
        Vector3f nWorld = SphericalDirection(sinAlpha, cosAlpha, phi,
                                   -wcX, -wcY, -wc);
		Point3f pWorld = radius * Point3f(nWorld.x, nWorld.y, nWorld.z);

		Interaction it;
        it.interactPoint = pWorld;
        it.pError = gamma(5) * Vector3f::Abs(pWorld);
        it.normal = nWorld;

		//计算pdf,用cone的方式去计算
		//单位球条带面积：2πsinθdθ
		//cone的面积：∫[0,θmax]2πsinθdθ = 2π(1 - cosθmax)
		//所以cone上的p(ω) = 1 / (2π(1 - cosθmax))
		*pdf = 1.0f / (Pi * 2.0f * (1.0f - cosThetaMax));

		return it;
	}

	Float Sphere::Pdf(const Interaction &ref, const Vector3f &wi) const
	{
		Point3f pCenter = mTransform->ObjectToWorldPoint(Vector3f::zero);
		// Return uniform PDF if point is inside sphere
    	Point3f pOrigin =
        	OffsetRayOrigin(ref.interactPoint, ref.pError, ref.normal, pCenter - ref.p);
    	if (Vector3f::DistanceSquare(pOrigin, pCenter) <= radius * radius)
        	return Shape::Pdf(ref, wi);

		Float sinThetaMax2 = radius * radius / Vector3f::DistanceSquare(ref.p, pCenter);
		Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
		return UniformConePdf(cosThetaMax);
	}
}
