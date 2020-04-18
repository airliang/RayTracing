#include "robject.h"
#include "sphere.h"
#include "material.h"

namespace AIR
{
	RObject::~RObject()
	{

	}

	RObject* RObject::CreateRObject(Shape::ShapeType shapeType, const GeometryParam& param, const Vector3f& position, const Vector3f& scale, const Quaternion& rotation)
	{
		RObject* pRObject = new RObject();
		Transform transform;
		transform.SetPosition(position);
		transform.SetScale(scale); 
		transform.SetRotation(rotation); 
		switch (shapeType) 
		{
		case AIR::Shape::shape_sphere:
			pRObject->shape = Sphere::CreateSphere(param, &transform);
			break;
		case AIR::Shape::shape_cylinder:
			break;
		default:
			break;
		}
		
		pRObject->mTransform = transform;
		return pRObject;
	}

	Bounds3f RObject::WorldBound() const
	{
		if (shape == nullptr)
		{
			return Bounds3f();
		}
		return mTransform.ObjectToWorldBound(shape->ObjectBound());
	}

	bool RObject::Intersect(const Ray &r, Interaction* pInteract) const
	{
		Float tHit = 0;
		if (!shape->Intersect(r, &tHit, pInteract))
		{
			return false;
		}
		r.tMax = tHit;
		return true;
	}

	bool RObject::IntersectP(const Ray &r) const
	{
		return shape->IntersectP(r);
	}

	void RObject::ComputeScatteringFunctions(
		Interaction* isect, MemoryArena& arena, TransportMode mode,
		bool allowMultipleLobes) const {
		if (material)
			material->ComputeScatteringFunctions(isect, arena, mode,
				allowMultipleLobes);
	}
}