#include "mirrormaterial.h"
#include "bsdf.h"

namespace AIR
{
	void MirrorMaterial::ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena,
		TransportMode mode, bool allowMultipleLobes) const
	{
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
		Spectrum specular = Kr->Evaluate(*si).Clamp();
		
		if (!specular.IsBlack())
		{
			Fresnel* fresnel = ARENA_ALLOC(arena, FresnelNoOp)();
			si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(specular, fresnel));
		}
		
	}
}
