//scene objects class, has own transform and shape.

#pragma once
#include "transform.h"
#include "shape.h"
#include "geometryparam.h"
namespace AIR
{
	class Interaction;
	class MemoryArena;
	class Material;
	class AreaLight;
	class RObject
	{
	public:
		RObject()
		{

		}
		virtual ~RObject();

		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray &r, Interaction *) const;
		virtual bool IntersectP(const Ray &r) const;
		
		//initializes representations of the light-scattering properties of the 
		//material at the intersection point on the surface.
		virtual void ComputeScatteringFunctions(Interaction* isect,
			MemoryArena& arena, TransportMode mode,
			bool allowMultipleLobes) const;

		static RObject* CreateRObject(Shape::ShapeType shapeType, const GeometryParam& param, const Vector3f& position, const Vector3f& scale, const Quaternion& rotation);

		const AreaLight* GetAreaLight() const
		{
			return areaLight.get();
		}

		const Material* GetMaterial() const
		{
			return material.get();
		}
	private:
		
		std::shared_ptr<Shape> shape;
		std::shared_ptr<Material> material;
		std::shared_ptr<AreaLight> areaLight;

	protected:
		Transform mTransform;
	};
}
