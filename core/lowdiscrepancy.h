#pragma once
#include <cstdint>
#include <vector>
#include "../RayTracing.h"
#include "sampling.h"

namespace AIR
{
	//constexpr常量表达式，可用于数值长度
	static constexpr int PrimeTableSize = 1000;
	//存储了PrimeTableSize个质数的数组
	extern const int Primes[PrimeTableSize];
	//每个质数在数组中的起始位置索引
	extern const int PrimeSums[PrimeTableSize];

	// Low Discrepancy Inline Functions
	inline uint32_t ReverseBits32(uint32_t n) {
		n = (n << 16) | (n >> 16);
		n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
		n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
		n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
		n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
		return n;
	}

	inline uint64_t ReverseBits64(uint64_t n) {
		uint64_t n0 = ReverseBits32((uint32_t)n);
		uint64_t n1 = ReverseBits32((uint32_t)(n >> 32));
		return (n0 << 32) | n1;
	}

	//反转位数
	//@param baseIndex base的索引
	//@param a         要反转的数字
	//@return    反转后的小数
	//例如a = 5 baseIndex = 0
	//那么baseIndex = 0对应的是二进制即base = 2
	// a = 101 inverse = 0.101 = 5/8
	Float RadicalInverse(int baseIndex, uint64_t a);

	/** 注释抄袭自paladin
 * 反转函数
 * base 为进制数
 * @param  inverse 需要反转的数字
 * @param  nDigits 需要反转的前nDigits位
 * @return         返回 base进制数 inverse 的前 nDigits 位反转之后的数字
 * 例如，InverseRadicalInverse<10>(1234567, 3)的返回值为765
 */
	template <int base>
	inline uint64_t InverseRadicalInverse(uint64_t inverse, int nDigits) {
		uint64_t index = 0;
		for (int i = 0; i < nDigits; ++i) {
			uint64_t digit = inverse % base;
			inverse /= base;
			index = index * base + digit;
		}
		return index;
	}

	//求PrimeTableSize个质数的digit的乱序数组
	std::vector<uint16_t> ComputeRadicalInversePermutations(RNG& rng);

	//继续以2进制为例，
	//2进制[0, 1]乱序后是[1, 0]
	//a = 5 baseIndex = 0
	//a = 101 inverse = 0.p[1]p[0]p[1] = 0.010 = 1/4
	//@param perm  乱序后的digit数组
	//例如:
	//   2     3         5 …… prime base
	//[0,1,0,1,2,0,1,2,3,4,……] before permutation
	//[1,0,0,2,1,2,0,4,3,1,……] after  permutation
	Float ScrambledRadicalInverse(int baseIndex, uint64_t a, const uint16_t* perm);
}
