#pragma once
#include "geometry.h"
#include "spectrum.h"
#include "sampler.h"
#include "memory.h"

namespace AIR
{
	class MediumInteraction;
	//Henyey and Greenstein Phase Function
	inline Float PhaseHG(Float cosTheta, Float g) 
	{
		Float denom = 1 + g * g + 2 * g * cosTheta;
		return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
	}

	//phase function
	//distribution of wi as bsdf function
	//∫p(wo, wi)dwi = 1
	//均匀分布的话
	//p(wo, wi) = 1/4π
	class PhaseFunction
	{
	public:
		//returns the value of the phase function for the given pair of directions.
		virtual Float p(const Vector3f& wo, const Vector3f& wi) const = 0;

		//根据wo采样wi，并返回p
		virtual Float Sample_p(const Vector3f& wo, Vector3f* wi, const Point2f& sample) const = 0;
	};

	class HenyeyGreenstein : public PhaseFunction 
	{
	public:
		HenyeyGreenstein(Float g) : g(g) { }
		Float p(const Vector3f& wo, const Vector3f& wi) const;
		Float Sample_p(const Vector3f& wo, Vector3f* wi, const Point2f& sample) const;

	private:
		const Float g;
	};


	class Medium
	{
	public:
		virtual ~Medium() { }
		//衰减公式
		virtual Spectrum Tr(const Ray& ray, Sampler& sampler) const = 0;
		virtual Spectrum Sample(const Ray& ray, Sampler& sampler,
			MemoryArena& arena, MediumInteraction* mi) const = 0;

	private:
	};


	struct MediumInterface 
	{
		MediumInterface() : inside(nullptr), outside(nullptr) {}
		// MediumInterface Public Methods
		MediumInterface(const Medium* medium) : inside(medium), outside(medium) {}
		MediumInterface(const Medium* inside, const Medium* outside)
			: inside(inside), outside(outside) {}
		bool IsMediumTransition() const { return inside != outside; }
		const Medium* inside, * outside;
	};
}
