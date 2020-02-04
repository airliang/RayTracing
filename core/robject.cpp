#include "robject.h"
#include "sphere.h"

namespace AIR
{
	RObject::~RObject()
	{

	}

	RObject* RObject::CreateRObject(Shape::ShapeType shapeType, const GeometryParam& param, const Vector3f& position, const Vector3f& scale, const Quaternion& rotation)
	{
		Shape* pShape = nullptr;
		Transform transform;
		transform.SetPosition(position);
		transform.SetScale(scale); 
		transform.SetRotation(rotation); 
		switch (shapeType) 
		{
		case AIR::Shape::shape_sphere:
			pShape = Sphere::CreateSphere(param, &transform).get();
			break;
		case AIR::Shape::shape_cylinder:
			break;
		default:
			break;
		}
		RObject* pRObject = new RObject(pShape);
		pRObject->mTransform = transform;
		return pRObject;
	}

	Bounds3f RObject::WorldBound() const
	{
		if (mShape == nullptr)
		{
			return Bounds3f();
		}
		return mTransform.ObjectToWorldBound(mShape->ObjectBound());
	}

	bool RObject::Intersect(const Ray &r, Interaction* pInteract) const
	{
		Float tHit = 0;
		if (!mShape->Intersect(r, &tHit, pInteract))
		{
			return false;
		}
		r.tMax = tHit;
		return true;
	}

	bool RObject::IntersectP(const Ray &r) const
	{
		return mShape->IntersectP(r);
	}
}