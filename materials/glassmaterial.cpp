#include "glassmaterial.h"
#include "microfacet.h"
#include "bsdf.h"

namespace AIR
{
	GlassMaterial::GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
		const std::shared_ptr<Texture<Spectrum>>& Kt,
		const std::shared_ptr<Texture<Float>>& uRoughness,
		const std::shared_ptr<Texture<Float>>& vRoughness,
		const std::shared_ptr<Texture<Float>>& index,
		const std::shared_ptr<Texture<Float>>& bumpMap,
		bool remapRoughness) : Kr(Kr)
		, Kt(Kt)
		, uRoughness(uRoughness)
		, vRoughness(vRoughness)
		, index(index)
		, bumpMap(bumpMap)
		, remapRoughness(remapRoughness)
	{

	}

	void GlassMaterial::ComputeScatteringFunctions(Interaction* si,
		MemoryArena& arena, TransportMode mode,
		bool allowMultipleLobes) const
	{
		Float eta = index->Evaluate(*si);
		Float urough = uRoughness->Evaluate(*si);
		Float vrough = vRoughness->Evaluate(*si);
		Spectrum R = Kr->Evaluate(*si).Clamp();
		Spectrum T = Kt->Evaluate(*si).Clamp();
		// Initialize _bsdf_ for smooth or rough dielectric
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, eta);

		if (R.IsBlack() && T.IsBlack()) 
			return;

		//粗糙度是0就是perfect specular
		bool isSpecular = urough == 0 && vrough == 0;
		if (isSpecular && allowMultipleLobes) 
		{
			si->bsdf->Add(
				ARENA_ALLOC(arena, FresnelSpecular)(R, T, 1.f, eta, mode));
		}
		else 
		{
			if (remapRoughness) 
			{
				urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
				vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
			}
			MicrofacetDistribution* distrib =
				isSpecular ? nullptr
				: ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(
					urough, vrough);
			if (!R.IsBlack()) 
			{
				//Fresnel决定多少被反射了
				Fresnel* fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, eta);
				if (isSpecular)
					si->bsdf->Add(
						ARENA_ALLOC(arena, SpecularReflection)(R, fresnel));
				else
					si->bsdf->Add(ARENA_ALLOC(arena, MicrofacetReflection)(
						R, distrib, fresnel));
			}
			if (!T.IsBlack()) 
			{
				if (isSpecular)
					si->bsdf->Add(ARENA_ALLOC(arena, SpecularTransmission)(
						T, 1.f, eta));
				else
					si->bsdf->Add(ARENA_ALLOC(arena, MicrofacetTransmission)(
						T, distrib, 1.f, eta, mode));
			}
		}
	}
}