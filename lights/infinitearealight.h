#pragma once
#include "light.h"

namespace AIR
{
	class InfiniteAreaLight : public AreaLight
	{
	public:
		InfiniteAreaLight(const Transform& LightToWorld,
			const Spectrum& L, int nSamples, const std::string& texmap) :
			AreaLight((int)LightFlags::Area, LightToWorld, nSamples)
		{

		}

		virtual Spectrum LiEscape(const RayDifferential& r) const;
	};
}
