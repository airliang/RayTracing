#include "distantlight.h"
#include "scene.h"

namespace AIR
{

	Spectrum DistantLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, Float* pdf, VisibilityTester* vis) const
	{
		*wi = wLight;
		*pdf = 1.0f;

		//��worldbound��������ģ��ù�Դ��λ��
		Point3f lightPos = ref.interactPoint + wLight * (2 * worldRadius);
		*vis = VisibilityTester(ref, Interaction(lightPos, ref.time));

		return L;
	}

	Spectrum DistantLight::Power() const
	{
		return 2 * Pi * worldRadius * worldRadius * L;
	}

	void DistantLight::Preprocess(const Scene& scene)
	{
		scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
	}
}
