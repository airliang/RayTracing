// RayTracing.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
#include <iostream>
#include <cmath>
// TODO: 在此处引用程序需要的其他标头。
typedef float Float;

enum class TransportMode { Radiance, Importance };

//在栈里alloc，不需要free
#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))


#if defined(__GNUC__)
#define F_INLINE                inline __attribute__((always_inline))
#define NO_INLINE               __attribute__((noinline))
#define EXPECT_TAKEN(a)        __builtin_expect(!!(a), true)
#define EXPECT_NOT_TAKEN(a)    __builtin_expect(!!(a), false)
#elif defined(_MSC_VER)
#define F_INLINE                __forceinline
#define NO_INLINE               __declspec(noinline)
#define MM_ALIGN16             __declspec(align(16))
#define EXPECT_TAKEN(a)        (a)
#define EXPECT_NOT_TAKEN(a)    (a)
#else
#error Unsupported compiler!
#endif

#if defined(_WIN32) || defined(_WIN64)
#define IS_WINDOWS
#define NOMINMAX
#endif

#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif

#ifndef HAVE__ALIGNED_MALLOC
#define HAVE__ALIGNED_MALLOC
#endif


