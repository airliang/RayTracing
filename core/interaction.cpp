#include "interaction.h"

namespace AIR
{
	void Interaction::SetGeometryShading(const Vector3f &dpdus, const Vector3f &dpdvs, const Vector3f &dndus, const Vector3f &dndvs, bool orientationIsAuthoritative)
	{
		shading.dndu = dndus;
		shading.dndv = dndvs;
		shading.dpdu = dpdus;
		shading.dpdv = dpdvs;

		shading.n = Vector3f::Normalize(Vector3f::Cross(shading.dpdu, shading.dpdv));
    }
}
