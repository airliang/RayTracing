#include "matte.h"
#include "bsdf.h"

namespace AIR
{
	void MatteMaterial::ComputeScatteringFunctions(Interaction *si, MemoryArena &arena, TransportMode mode, bool allowMultipleLobes) const
	{
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);


	}
}
