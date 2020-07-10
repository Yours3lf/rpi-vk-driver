
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${EXTERNAL_SYSROOT}/include)
include_directories(${EXTERNAL_SYSROOT}/include/drm)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(CMAKE_VERBOSE_MAKEFILE TRUE)
	add_definitions(-DDEBUG_BUILD)
    add_compile_options(-Wall)
else()
    # filter as-needed here
    add_compile_options(-Wall) #-Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable)
    #add_compile_options(-Werror)
endif()

link_directories(
    ${EXTERNAL_SYSROOT}/lib
    ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader
)

set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
