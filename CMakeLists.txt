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
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openexr")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/math)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shapes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filters)
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
#	core/shape.h
#	core/robject.h
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
    )

set(SHAPES_SOURCES  shapes/geometryparam.h
					shapes/sphere.h
					shapes/sphere.cpp
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

set(SOURCES 
    RayTracing.h
	RayTracing.cpp
    ${CORE_SOURCES}
	${SHAPES_SOURCES}
	${MATH_SOURCES}
	${SAMPLER_SOURCES}
	${FILTER_SOURCES}
	)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
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
add_compile_options(-std=c++14)
