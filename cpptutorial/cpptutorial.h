#pragma once

#include <stdio.h>

typedef unsigned char* byte_pointer;

void show_bytes(byte_pointer start, int len) {
	int i;
	for (i = 0; i < len; i++)
		printf(" %.2x", start[i]);
	printf("\n");

}

void show_int(int x) 
{
	show_bytes((byte_pointer)&x, sizeof(int));
}

void show_float(float x) {
    show_bytes((byte_pointer)&x, sizeof(float));
}

void show_pointer(void* x) {
	show_bytes((byte_pointer)&x, sizeof(void*));
}

void inplace_swap(int* x, int* y) {
	*y = *x ^ *y; /* Step 1 */
	*x = *x ^ *y; /* Step 2 */
	*y = *x ^ *y; /* Step 3 */
}

int fun1(unsigned word) 
{
	return (int)((word << 24) >> 24);
}
int fun2(unsigned word) 
{
	return ((int)word << 24) >> 24;
}

void test_showbytes()
{
	int val = 0x87654321;
	byte_pointer valp = (byte_pointer)&val;
	show_bytes(valp, 1); /* A. */
	show_bytes(valp, 2); /* B. */
	show_bytes(valp, 3); /* C. */

	val = 3510593;
	show_int(val);

	show_float((float)val);

	const char* s = "abcdef";
	show_bytes((byte_pointer)s, strlen(s));

	val = 0x00000076;
	int shift = fun2(val);
	printf("shift = %d \n", shift);
	show_int(shift);

	val = 0x000000C9;
	shift = fun2(val);
	printf("shift = %d \n", shift);
	show_int(shift);
}

int div16(int x)
{
	int bias = (x >> 31) & 0xF;
	//如果x是负数，那么bias = 15，算出的结果往0靠近。
	return (x + bias) >> 4;
}


class A
{
public:
	A() : x(0)
	{
		std::cout << "default construct" << std::endl;
	}

	A(A& other) : x(other.x)
	{
		std::cout << "copy construct" << std::endl;
	}

	A(A&& other) : x(other.x)
	{
		std::cout << "move construct" << std::endl;
	}
	
private:
	int x;
};

void test_rightvalue()
{
	int&& x = 5;

	A a;
	A b = std::move(a);
	A* c = new A();

	int y = 3;
	//int y = 3;
	inplace_swap(&y, &y);
	
	std::cout << "after inplace_swap, y=" << y << std::endl;

	y = 0x87654321 & 0x000000FF;
	printf("y = %08x\n", y);

	y = ~0x87654300 | 0x00000021 ;
	printf("y = %08x\n", y);
}

void test_operators()
{
	int x = 17;
	printf("y = %d\n", div16(x));
	int y = 17 / 16;
	printf("operator div y = %d\n", y);

	printf("y = %d\n", div16(-x));
	y = (-17) / 16;
	printf("operator div y = %d\n", y);
}

struct SamplerState
{
	size_t sampler_filter0 : 2;
	size_t sampler_filter1 : 2;
	size_t sampler_filter2 : 2;
	size_t sampler_filter3 : 2;
	size_t sampler_filter4 : 2;
	size_t sampler_filter5 : 2;
	size_t sampler_filter6 : 2;
	size_t sampler_filter7 : 2;
	// test_alphaShadow
	size_t sampler_filter8 : 2;
	size_t sampler_filter9 : 2;
	size_t sampler_filter10 : 2;
	size_t sampler_filter11 : 2;
	// test_end

	size_t sampler_adr0 : 3;
	size_t sampler_adr1 : 3;
	size_t sampler_adr2 : 3;
	size_t sampler_adr3 : 3;
	size_t sampler_adr4 : 3;
	size_t sampler_adr5 : 3;
	size_t sampler_adr6 : 3;
	size_t sampler_adr7 : 3;

	// test_alphaShadow
	size_t sampler_adr8 : 2;
	size_t sampler_adr9 : 2;
	size_t sampler_adr10 : 2;
	size_t sampler_adr11 : 2;
	// test_end

	size_t sampler_srgb0 : 1;
	size_t sampler_srgb1 : 1;
	size_t sampler_srgb2 : 1;
	size_t sampler_srgb3 : 1;
	size_t sampler_srgb4 : 1;
	size_t sampler_srgb5 : 1;
	size_t sampler_srgb6 : 1;
	size_t sampler_srgb7 : 1;
	size_t sampler_srgb8 : 1;
	size_t sampler_srgb9 : 1;
	size_t sampler_srgb10 : 1;
	size_t sampler_srgb11 : 1;

	float sampler_miplodbias0;
	float sampler_miplodbias1;

	SamplerState()
	{
		memset(this, 0, sizeof(SamplerState));
	}
};
