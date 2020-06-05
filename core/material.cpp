#include "material.h"

namespace AIR
{
	void Material::Bump(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction *si)
	{
		//p'(u,v) = p(u,v) + d(u,v)n(u,v)   1式
		//对1式求u偏导
		//∂p'    ∂p(u,v)   d(u + Δu,v) - d(u,v)               ∂n
		//--- ≈ ------- + ---------------------n(u,v) + d(u,v)---
		//∂u       ∂u               Δu                        ∂u

		float displace = d->Evaluate(*si);

		//因为很多方法和SurfaceInteraction耦合起来，所以必须这样做
		SurfaceInteraction deltaSi = *si;
		//delta u
		float du = 0.5f * (std::abs(si->dudx) + std::abs(si->dudy));
		if (du == 0)
			du = 0.01f;
		deltaSi.interactPoint = si->interactPoint + du * si->shading.dpdu;
		deltaSi.uv = si->uv + Vector2(du, 0);
		deltaSi.normal = Vector3f::Normalize(Vector3f::Cross(si->shading.dpdu, si->shading.dpdv) + du * si->dndu);
		float uDisplace = d->Evaluate(deltaSi);

		//SurfaceInteraction deltaVsi = *si;
		float dv = 0.5f * (std::abs(si->dvdx) + std::abs(si->dvdy)); //delta v
		if (dv == 0)
			dv = 0.01f;
		deltaSi.interactPoint = si->interactPoint + dv * si->shading.dpdv;
		deltaSi.uv = si->uv + Vector2(0, dv);
		deltaSi.normal = Vector3f::Normalize(Vector3f::Cross(si->shading.dpdu, si->shading.dpdv) + dv * si->dndv);
		float vDisplace = d->Evaluate(deltaSi);

		
		Vector3f dpdu = si->shading.dpdu + si->shading.n * (uDisplace - displace) / du + displace * si->shading.dndu;
		Vector3f dpdv = si->shading.dpdv + si->shading.n * (uDisplace - displace) / dv + displace * si->shading.dndv;

		si->SetGeometryShading(dpdu, dpdv, si->shading.dndu, si->shading.dndv, false);
	}
}
