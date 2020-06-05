#include "shape.h"

namespace AIR
{
	class Disk : public Shape
	{
	public:
		Disk(Transform* pTransform,
			const Float height,
			const Float radius,
			const Float innerRadius,
			const Float phiMax) : Shape(pTransform),
			height(height), radius(radius), innerRadius(innerRadius), phiMax(phiMax)
		{

		}

		Bounds3f ObjectBound() const;

		Float Area() const;

		bool Intersect(const Ray& r, Float* tHit, SurfaceInteraction* isect) const;

		virtual Interaction Sample(const Point2f& u, Float* pdf) const;

		bool IntersectP(const Ray& ray) const;

	private:
		const Float height;
		const Float radius;
		const Float innerRadius;
		const Float phiMax;
	};
}
