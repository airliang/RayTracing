#include "matte.h"
#include "bsdf.h"
#include "memory.h"

namespace AIR
{
	void MatteMaterial::ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena, TransportMode mode, bool allowMultipleLobes) const
	{
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);

		Spectrum r = Kd->Evaluate(*si).Clamp();
		Float sig = Clamp(sigma->Evaluate(*si), 0, 90);
		if (!r.IsBlack()) {
			if (sig == 0)
				si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(r));
			else
				si->bsdf->Add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
		}
	}
}
