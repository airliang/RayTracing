# CMakeList.txt: RayTracing 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
set(SOURCES
    geometryparam.h
	sphere.h
	sphere.cpp
    )


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../math)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)

# TODO: 如有需要，请添加测试并安装目标。
