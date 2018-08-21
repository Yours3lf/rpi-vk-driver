cmake_minimum_required (VERSION 3.0)

project (rpi-vulkan-driver)

#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_VERBOSE_MAKEFILE ON)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	message("Debug build")
	add_definitions(-DDEBUG_BUILD)
endif()

set (EXTERNAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/)
link_directories(${EXTERNAL_PATH}/lib)
include_directories(${EXTERNAL_PATH}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(brcm)
add_subdirectory(driver)
add_subdirectory(test)
