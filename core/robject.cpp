#include "robject.h"
#include "sphere.h"
#include "material.h"

namespace AIR
{
	Primitive::~Primitive()
	{

	}

	/*
	Primitive* Primitive::CreatePrimitive(Shape::ShapeType shapeType, const GeometryParam& param, Transform* pTransform)
	{
		Primitive* pRObject = new Primitive();

		switch (shapeType) 
		{
		case AIR::Shape::shape_sphere:
			pRObject->shape = Sphere::CreateSphere(param, pTransform);
			break;
		case AIR::Shape::shape_cylinder:
			break;
		default:
			break;
		}
		
		pRObject->mTransform = pTransform;
		return pRObject;
	}
	*/

	Bounds3f Primitive::WorldBound() const
	{
		if (shape == nullptr)
		{
			return Bounds3f();
		}
		return mTransform->ObjectToWorldBound(shape->ObjectBound());
	}

	bool Primitive::Intersect(const Ray &r, SurfaceInteraction* pInteract) const
	{
		Float tHit = 0;
		if (!shape->Intersect(r, &tHit, pInteract))
		{
			return false;
		}
		r.tMax = tHit;
		pInteract->primitive = this;

		//如果媒介是有不同过渡的，那么用这个primtive的mediumInterface
		if (mediumInterface.IsMediumTransition())
			pInteract->mediumInterface = mediumInterface;
		else   //否则优先用ray携带的medium
			pInteract->mediumInterface = MediumInterface(r.medium);

		return true;
	}

	bool Primitive::IntersectP(const Ray &r) const
	{
		return shape->IntersectP(r);
	}

	void Primitive::ComputeScatteringFunctions(
		SurfaceInteraction* isect, MemoryArena& arena, TransportMode mode,
		bool allowMultipleLobes) const {
		if (material)
			material->ComputeScatteringFunctions(isect, arena, mode,
				allowMultipleLobes);
	}
}