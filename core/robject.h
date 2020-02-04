//scene objects class, has own transform and shape.

#pragma once
#include "transform.h"
#include "shape.h"
#include "geometryparam.h"
namespace AIR
{
	class RObject
	{
	public:
		RObject(Shape* pShape) : mShape(pShape)
		{

		}
		~RObject();

		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray &r, Interaction *) const;
		virtual bool IntersectP(const Ray &r) const;
		//virtual const AreaLight *GetAreaLight() const = 0;
		//virtual const Material *GetMaterial() const = 0;

		static RObject* CreateRObject(Shape::ShapeType shapeType, const GeometryParam& param, const Vector3f& position, const Vector3f& scale, const Quaternion& rotation);
	private:
		
		Shape* mShape;

	protected:
		Transform mTransform;
	};
}
