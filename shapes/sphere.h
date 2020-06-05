#pragma once

#include "shape.h"
#include "geometryparam.h"


namespace AIR
{
	class Integrator;
	//按z向上的坐标系来定义球，
	//方便采样时的计算
	class Sphere : public Shape
	{
	public:
		Sphere(Float radius, Float yMin, Float yMax, Float phiMax, Transform* pTransform) : Shape(pTransform)
			, radius(radius)
			, yMin(Clamp(std::min(yMin, yMax), -radius, radius))
			, yMax(Clamp(std::max(yMin, yMax), -radius, radius))
			, phiMax(phiMax)
			, thetaMax(std::acos(Clamp(std::min(yMin, yMax) / radius, -1, 1)))
			, thetaMin(std::acos(Clamp(std::max(yMin, yMax) / radius, -1, 1)))
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

		Interaction Sample(const Point2f& u, Float* pdf) const;

		Interaction Sample(const Interaction& ref, const Point2f& u,
			Float* pdf) const;

		Float Area() const
		{
			return phiMax * radius * (yMax - yMin);
		}

		Float Pdf(const Interaction& ref, const Vector3f &wi) const;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;
    private:
		Float radius;
		Float thetaMin, thetaMax;
		Float phiMax;
		Float yMax;
		Float yMin;

	public:
		static std::shared_ptr<Sphere> CreateSphere(const GeometryParam& param, Transform* pTransform);
	};
}