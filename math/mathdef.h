#pragma once 
#include "../RayTracing.h"
#include <algorithm>
namespace AIR
{
	// Global Constants
#ifdef _MSC_VER
#define MaxFloat std::numeric_limits<Float>::max()
#define Infinity std::numeric_limits<Float>::infinity()
#else
static constexpr Float MaxFloat = std::numeric_limits<Float>::max();
static constexpr Float Infinity = std::numeric_limits<Float>::infinity();
#endif
#ifdef _MSC_VER
#define MachineEpsilon (std::numeric_limits<Float>::epsilon() * 0.5)
#else
static constexpr Float MachineEpsilon =
    std::numeric_limits<Float>::epsilon() * 0.5;
#endif
	static Float ShadowEpsilon = 0.0001f;
	static Float Pi = 3.14159265358979323846;
	static Float InvPi = 0.31830988618379067154;
	static Float Inv2Pi = 0.15915494309189533577;
	static Float Inv4Pi = 0.07957747154594766788;
	static Float PiOver2 = 1.57079632679489661923;
	static Float PiOver4 = 0.78539816339744830961;
	static Float Sqrt2 = 1.41421356237309504880;


	// Global Inline Functions
	inline uint32_t FloatToBits(float f) {
		uint32_t ui;
		memcpy(&ui, &f, sizeof(float));
		return ui;
	}

	inline float BitsToFloat(uint32_t ui) {
		float f;
		memcpy(&f, &ui, sizeof(uint32_t));
		return f;
	}

	inline uint64_t FloatToBits(double f) {
		uint64_t ui;
		memcpy(&ui, &f, sizeof(double));
		return ui;
	}

	inline double BitsToFloat(uint64_t ui) {
		double f;
		memcpy(&f, &ui, sizeof(uint64_t));
		return f;
	}

	inline float NextFloatUp(float v) {
		// Handle infinity and negative zero for _NextFloatUp()_
		if (std::isinf(v) && v > 0.) return v;
		if (v == -0.f) v = 0.f;

		// Advance _v_ to next higher float
		uint32_t ui = FloatToBits(v);
		if (v >= 0)
			++ui;
		else
			--ui;
		return BitsToFloat(ui);
	}

	inline float NextFloatDown(float v) {
		// Handle infinity and positive zero for _NextFloatDown()_
		if (std::isinf(v) && v < 0.) return v;
		if (v == 0.f) v = -0.f;
		uint32_t ui = FloatToBits(v);
		if (v > 0)
			--ui;
		else
			++ui;
		return BitsToFloat(ui);
	}

	inline double NextFloatUp(double v, int delta = 1) {
		if (std::isinf(v) && v > 0.) return v;
		if (v == -0.f) v = 0.f;
		uint64_t ui = FloatToBits(v);
		if (v >= 0.)
			ui += delta;
		else
			ui -= delta;
		return BitsToFloat(ui);
	}

	inline double NextFloatDown(double v, int delta = 1) {
		if (std::isinf(v) && v < 0.) return v;
		if (v == 0.f) v = -0.f;
		uint64_t ui = FloatToBits(v);
		if (v > 0.)
			ui -= delta;
		else
			ui += delta;
		return BitsToFloat(ui);
	}

	inline Float gamma(int n) {
		return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
	}

	inline Float GammaCorrect(Float value) {
		if (value <= 0.0031308f) return 12.92f * value;
		return 1.055f * std::pow(value, (Float)(1.f / 2.4f)) - 0.055f;
	}

	inline Float InverseGammaCorrect(Float value) {
		if (value <= 0.04045f) return value * 1.f / 12.92f;
		return std::pow((value + 0.055f) * 1.f / 1.055f, (Float)2.4f);
	}

	template <typename T, typename U, typename V>
	inline T Clamp(T val, U low, V high) {
		if (val < low)
			return low;
		else if (val > high)
			return high;
		else
			return val;
	}

	template <typename T>
	inline T Mod(T a, T b) {
		T result = a - (a / b) * b;
		return (T)((result < 0) ? result + b : result);
	}

	template <>
	inline Float Mod(Float a, Float b) {
		return std::fmod(a, b);
	}

	inline Float Radians(Float deg) { return (Pi / 180) * deg; }

	inline Float Degrees(Float rad) { return (180 / Pi) * rad; }

	inline Float Log2(Float x) {
		const Float invLog2 = 1.442695040888963387004650940071;
		return std::log(x) * invLog2;
	}

	inline int Log2Int(uint32_t v) 
	{
#if defined(_MSC_VER)
		unsigned long lz = 0;
		if (_BitScanReverse(&lz, v)) 
			return lz;
		return 0;
#else
		return 31 - __builtin_clz(v);
#endif
	}

	inline int Log2Int(int32_t v) 
	{ 
		return Log2Int((uint32_t)v); 
	}

	template <typename T>
	inline bool IsPowerOf2(T v) {
		return v && !(v & (v - 1));
	}

	inline int32_t RoundUpPow2(int32_t v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return v + 1;
	}

	inline int64_t RoundUpPow2(int64_t v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		return v + 1;
	}

	inline Float Lerp(Float t, Float v1, Float v2) { return (1 - t) * v1 + t * v2; }

	//binary search
	template <typename Predicate>
	int FindInterval(int size, const Predicate &pred) 
	{
		int first = 0, len = size;
		while (len > 0) 
		{
			int half = len >> 1, middle = first + half;
			// Bisect range based on value of _pred_ at _middle_
			if (pred(middle)) 
			{
				first = middle + 1;
				len -= half + 1;
			}
			else
				len = half;
		}
		return Clamp(first - 1, 0, size - 2);
	}

	inline bool SolveLinearSystem2x2(const Float A[2][2], const Float B[2], Float* x0,
		Float* x1) 
	{
		Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
		if (std::abs(det) < 1e-10f) return false;
		*x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
		*x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
		if (std::isnan(*x0) || std::isnan(*x1)) 
			return false;
		return true;
	}
};


