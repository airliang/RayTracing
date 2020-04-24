#include "plasticmaterial.h"
#include "bsdf.h"

namespace AIR
{
	void PlasticMaterial::ComputeScatteringFunctions(Interaction* si, MemoryArena& arena,
		TransportMode mode, bool allowMultipleLobes) const
	{
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
		//evaluate the diffuse spectrum at the current interaction point
		Spectrum diffuse = kd->Evaluate(*si).Clamp();

		//only if the diffuse is not black will add the bsdf
		if (!diffuse.IsBlack())
		{
			si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(diffuse));
		}

		//evaluate the diffuse spectrum at the current interaction point
		Spectrum specular = ks->Evaluate(*si).Clamp();
		if (!specular.IsBlack())
		{
			//1.0 Õæ¿ÕµÄÕÛÉäÂÊ
			//1.5 plastic's refraction index
			Fresnel* fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, 1.5f);

			Float rough = roughness->Evaluate(*si);
			if (remapRoughness)
				rough = TrowbridgeReitzDistribution::RoughnessToAlpha(rough);
			MicrofacetDistribution* distrib =
				ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(rough, rough);

			si->bsdf->Add(ARENA_ALLOC(arena, MicrofacetReflection)(specular, distrib, fresnel));
		}

	}
}
