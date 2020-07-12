#pragma once
#include <cstdint>
#include <vector>
#include "../RayTracing.h"
#include "sampling.h"

namespace AIR
{
	//constexpr�������ʽ����������ֵ����
	static constexpr int PrimeTableSize = 1000;
	//�洢��PrimeTableSize������������
	extern const int Primes[PrimeTableSize];
	//ÿ�������������е���ʼλ������
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

	//��תλ��
	//@param baseIndex base������
	//@param a         Ҫ��ת������
	//@return    ��ת���С��
	//����a = 5 baseIndex = 0
	//��ôbaseIndex = 0��Ӧ���Ƕ����Ƽ�base = 2
	// a = 101 inverse = 0.101 = 5/8
	Float RadicalInverse(int baseIndex, uint64_t a);

	/** ע�ͳ�Ϯ��paladin
 * ��ת����
 * base Ϊ������
 * @param  inverse ��Ҫ��ת������
 * @param  nDigits ��Ҫ��ת��ǰnDigitsλ
 * @return         ���� base������ inverse ��ǰ nDigits λ��ת֮�������
 * ���磬InverseRadicalInverse<10>(1234567, 3)�ķ���ֵΪ765
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

	//��PrimeTableSize��������digit����������
	std::vector<uint16_t> ComputeRadicalInversePermutations(RNG& rng);

	//������2����Ϊ����
	//2����[0, 1]�������[1, 0]
	//a = 5 baseIndex = 0
	//a = 101 inverse = 0.p[1]p[0]p[1] = 0.010 = 1/4
	//@param perm  ������digit����
	//����:
	//   2     3         5 ���� prime base
	//[0,1,0,1,2,0,1,2,3,4,����] before permutation
	//[1,0,0,2,1,2,0,4,3,1,����] after  permutation
	Float ScrambledRadicalInverse(int baseIndex, uint64_t a, const uint16_t* perm);
}
