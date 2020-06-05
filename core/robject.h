//scene objects class, has own transform and shape.

#pragma once
#include "transform.h"
#include "shape.h"
#include "geometryparam.h"
#include "medium.h"
namespace AIR
{
	class SurfaceInteraction;
	class MemoryArena;
	class Material;
	class AreaLight;
	class Primitive
	{
	public:
		Primitive() : mTransform(nullptr) {}

		Primitive(const std::shared_ptr<Shape>& shape,
			const std::shared_ptr<Material>& material,
			const std::shared_ptr<AreaLight>& areaLight,
			Transform* pTransform,
			const MediumInterface& mediumInterface) : shape(shape),
			material(material),
			areaLight(areaLight),
			mTransform(pTransform),
			mediumInterface(mediumInterface)
		{

		}
		virtual ~Primitive();

		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray &r, SurfaceInteraction *) const;
		virtual bool IntersectP(const Ray &r) const;
		
		//initializes representations of the light-scattering properties of the 
		//material at the intersection point on the surface.
		virtual void ComputeScatteringFunctions(SurfaceInteraction* isect,
			MemoryArena& arena, TransportMode mode,
			bool allowMultipleLobes) const;

		//static Primitive* CreatePrimitive(Shape::ShapeType shapeType, const GeometryParam& param, Transform* pTransform);

		const AreaLight* GetAreaLight() const
		{
			return areaLight.get();
		}

		const Material* GetMaterial() const
		{
			return material.get();
		}

		const Transform* GetTransform() const
		{
			return mTransform;
		}
	private:
		
		std::shared_ptr<Shape> shape;
		std::shared_ptr<Material> material;
		std::shared_ptr<AreaLight> areaLight;
		MediumInterface mediumInterface;
	protected:
		Transform* mTransform;
	};
}
