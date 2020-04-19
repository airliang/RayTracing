# CMakeList.txt: RayTracing 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project(RayTracing)
# 将源代码添加到此项目的可执行文件。
#add_executable (RayTracing "RayTracing.cpp" "RayTracing.h" "shapes/sphere.h" "shapes/sphere.cpp")

# TODO: 如有需要，请添加测试并安装目标。
# 包含子项目。
#add_subdirectory ("core")
#add_subdirectory ("math")
#add_subdirectory ("shapes")
#add_subdirectory ("filters")
#add_subdirectory ("samplers")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/math)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shapes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filters)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/scene)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/materials)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lights)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/integrators)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/samplers)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/OpenEXR/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/IlmBase/Half)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/IlmBase/Imath)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/OpenEXR)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/OpenEXR/IlmImf)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/IlmBase/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr/IlmBase/Iex)

set(CORE_SOURCES 
#    core/transform.h
	core/transform.cpp
	core/shape.cpp
	core/scene.cpp
	core/robject.cpp
#	core/interaction.h
#	core/application.h
#	core/efloat.h
	core/efloat.cpp
	core/main.cpp
	core/spectrum.cpp
#	core/spectrum.h
	core/camera.h
	core/camera.cpp
#	core/sampler.h
	core/sampler.cpp
#	core/rng.h
#	core/filter.h
	core/atomicfloat.h
	core/film.cpp
	core/imageio.cpp
	core/fileutil.cpp
	core/bsdf.cpp
	core/memory.cpp
	core/microfacet.cpp
	core/interaction.cpp
	core/texture.cpp
	core/mipmap.cpp
	core/parallelism.cpp
	core/light.cpp
	core/sampling.h
	core/sampling.cpp
	core/fresnelreflection.cpp
	core/integrator.cpp
	core/renderer.cpp
    )

set(SHAPES_SOURCES  shapes/geometryparam.h
					shapes/sphere.h
					shapes/sphere.cpp
					shapes/disk.cpp
					shapes/triangle.cpp
)

set(SAMPLER_SOURCES  samplers/stratified.h
					samplers/stratified.cpp
)

set(FILTER_SOURCES  filters/boxfilter.h
                    filters/trianglefilter.h
					filters/gaussianfilter.h
					filters/mitchellfilter.h
					filters/lanczossinfilter.h
)

set(MATH_SOURCES 
#    math/geometry.h
#	math/matrix.h
	math/matrix.cpp
#	math/mathdef.h
#	math/quaternion.h
	math/quaternion.cpp
)

set(SAMPLERS_SOURCES
    samplers/stratified.cpp
)

set(MATERIALS_SOURCES
    materials/matte.cpp
)

set(TEXTURES_SOURCES
    textures/imagetexture.cpp
	textures/bilerptexture.cpp
)

set(LIGHTS_SOURCES
    lights/pointlight.cpp
	lights/spotlight.cpp
	lights/distantlight.cpp
	lights/diffusearealight.cpp
	)

set(INTEGRATORS_SOURCES
    integrators/directlightingintegrator.cpp
	integrators/pathintegrator.cpp
    )

set(SCENE_SOURCES
    scene/bvhaccel.cpp
)

set(SOURCES 
    RayTracing.h
	RayTracing.cpp
    ${CORE_SOURCES}
	${SHAPES_SOURCES}
	${MATH_SOURCES}
	${SAMPLER_SOURCES}
	${FILTER_SOURCES}
	${MATERIALS_SOURCES}
	${TEXTURES_SOURCES}
	${LIGHTS_SOURCES}
	${INTEGRATORS_SOURCES}
	${SCENE_SOURCES}
	)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)


MESSAGE("Identifying the OS...")
if(WIN32)
  MESSAGE("This is Windows.")
elseif(APPLE)
  MESSAGE("This is MacOS.")
  SET(CMAKE_C_COMPILER "/usr/bin/gcc") # 默认c编译器
  SET(CMAKE_CXX_COMPILER "/usr/bin/g++") # 默认c++编译器
elseif(UNIX)
  MESSAGE("This is Linux.")
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
  SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/release")
elseif (CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/debug")
endif()

# 检查c++编译器标志，设置c++11支持变量
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("/std:c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("/std:c++0x" COMPILER_SUPPORTS_CXX0X)

# 使用变量设置编译标志
if(COMPILER_SUPPORTS_CXX11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++0x")
else()
message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

add_executable (RayTracing ${SOURCES})
if (CMAKE_BUILD_TYPE MATCHES Debug)
#target_link_libraries(RayTracing ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/debug/Half-2_4_d.lib)
#target_link_libraries(RayTracing ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/debug/Iex-2_4_d.lib)
#target_link_libraries(RayTracing ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/debug/IexMath-2_4_d.lib)
#target_link_libraries(RayTracing ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/debug/IlmThread-2_4_d.lib)
#target_link_libraries(RayTracing ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/debug/Imath-2_4_d.lib)
target_link_libraries(RayTracing Half IlmImf)
else ()
endif()
add_definitions(/std:c++14)
