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
