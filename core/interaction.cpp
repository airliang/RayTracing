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

	void Interaction::ComputeDifferentials(const RayDifferential& ray) const
	{
		//计算rayx的交点
		Float d = -Vector3f::Dot(normal, interactPoint);
		float ndo_x = Vector3f::Dot(normal, ray.rxOrigin);
		float ndd_x = Vector3f::Dot(normal, ray.rxDirection);
		float tx = (d - ndo_x) / ndd_x;

		Vector3f px = ray.rxOrigin + tx * ray.rxDirection;

		float ndo_y = Vector3f::Dot(normal, ray.ryOrigin);
		float ndd_y = Vector3f::Dot(normal, ray.ryDirection);
		float ty = (d - ndo_y) / ndd_y;

		Vector3f py = ray.ryOrigin + ty * ray.ryDirection;

		dpdx = px - interactPoint;
		dpdy = py - interactPoint;

		//求出px
		//p' = p + Δudpdu + Δvdpdv
		//解方程求Δu和Δv
		//px' - px = dpxduΔu + dpxdvΔv
		//py' - py = dpyduΔu + dpydvΔv
		//pz' - pz = dpzduΔu + dpzdvΔv
		//三个方程求两个变量，只需要选两个就可以了，怎么选？
		//取normal分量最小的两个来算，分量越小，dpdu的分量越大
		int dim[2] = { 0 };
		if ((std::abs(normal.x) > std::abs(normal.y)) && (std::abs(normal.x) > std::abs(normal.z)))
		{
			dim[0] = 1;
			dim[1] = 2;
		}
		else if (std::abs(normal.y) > std::abs(normal.z))
		{
			dim[0] = 0;
			dim[1] = 2;
		}
		else
		{
			dim[0] = 0;
			dim[1] = 1;
		}

		//解方程组求Δu和Δv
		//(p' - p)[dim[0]] = dpdu[dim[0]]Δu + dpdv[dim[0]]Δv
		//(p' - p)[dim[1]] = dpdu[dim[1]]Δu + dpdv[dim[1]]Δv
		Float A[2][2] = { { dpdu[dim[0]], dpdv[dim[0]] },
				  { dpdu[dim[1]], dpdv[dim[1]] } };
		Float Bx[2] = { px[dim[0]] - interactPoint[dim[0]], px[dim[1]] - interactPoint[dim[1]] };
		Float By[2] = { py[dim[0]] - interactPoint[dim[0]], py[dim[1]] - interactPoint[dim[1]] };

		if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx))
			dudx = dvdx = 0;
		if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy))
			dudy = dvdy = 0;
	}
}
