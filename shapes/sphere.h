#pragma once

#include "shape.h"
#include "geometryparam.h"


namespace AIR
{
	class Sphere : public Shape
	{
	public:
		Sphere(Float radius, Float thetaMin, Float thetaMax, Float phiMax, Transform* pTransform) : Shape(pTransform)
			, radius(radius)
			, thetaMin(thetaMin)
			, thetaMax(thetaMax)
			, phiMax(phiMax)
			, yMax(std::cos(thetaMin) * radius)
			, yMin(std::cos(thetaMax) * radius)
		{

		}

		~Sphere()
		{

		}

		virtual Bounds3f ObjectBound() const
		{
			return Bounds3f(Vector3f(-radius, yMin, -radius),
				Vector3f(radius, yMax, radius));
		}

		Float Area() const
		{
			return phiMax * radius * (yMax - yMin);
		}

		Float radius;
		Float thetaMin, thetaMax;
		Float phiMax;
		Float yMax;
		Float yMin;

	public:
		static std::shared_ptr<Sphere> CreateSphere(const GeometryParam& param, Transform* pTransform);

	//protected:
		bool Intersect(const Ray &r, Float *tHit, Interaction *isect) const;
	};
}