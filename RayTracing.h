﻿// RayTracing.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
#include <iostream>
#include <cmath>
// TODO: 在此处引用程序需要的其他标头。
typedef float Float;

//在栈里alloc，不需要free
#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))

#ifdef _MSC_VER

#endif

#ifdef __GNUC__

#endif

#if defined(_WIN32) || defined(_WIN64)
#define IS_WINDOWS
#endif
