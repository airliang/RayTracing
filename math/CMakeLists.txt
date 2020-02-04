# CMakeList.txt: RayTracing 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
set(SOURCES
    geometry.h
	matrix.h
	matrix.cpp
	mathdef.h
	quaternion.h
	quaternion.cpp
    )


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)


# TODO: 如有需要，请添加测试并安装目标。
