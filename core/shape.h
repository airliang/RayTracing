#pragma once
#include "geometry.h"
#include "interaction.h"
#include "transform.h"

namespace AIR
{

	class Shape
	{
	public:
		enum ShapeType
		{
			shape_sphere,
			shape_cylinder,
		};
	public:
		Shape(Transform* pTransform) : mTransform(pTransform)
		{

		}
		virtual ~Shape()
		{

		}

		virtual Bounds3f ObjectBound() const = 0;

		//ray is in object space
		virtual bool IntersectP(const Ray &ray) 
		{
			Float tHit = ray.tMax;
			Interaction isect;
			return false;
			//return Intersect(ray, &tHit, &isect);
		}

		Transform* mTransform;
		virtual bool Intersect(const Ray &r, Float *tHit, Interaction *isect) const = 0;
	protected:
		
	};
}
