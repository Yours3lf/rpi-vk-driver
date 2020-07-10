include_guard()

set(CMAKE_SYSTEM_NAME Linux)

# arch
IF(NOT TOOLCHAIN_TRIPLE)
    set(TOOLCHAIN_TRIPLE arm-linux-gnueabihf)
endif()

set(TOOLCHAIN_PREFIX ${TOOLCHAIN_TRIPLE}-)

message(STATUS "Triple ................. ${TOOLCHAIN_TRIPLE}")

STRING(REGEX REPLACE "^([a-zA-Z0-9]+).*" "\\1" target_arch "${TOOLCHAIN_TRIPLE}")
message(STATUS "Triple Arch ............ ${target_arch}")

set(CMAKE_SYSTEM_PROCESSOR ${target_arch})

# toolchain path
if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()

execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}g++
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(TOOLCHAIN_PATH ${BINUTILS_PATH} DIRECTORY)
get_filename_component(TOOLCHAIN_ROOT ${TOOLCHAIN_PATH} DIRECTORY)

# sysroot
if(NOT TARGET_SYSROOT)
    set(TARGET_SYSROOT "${TOOLCHAIN_ROOT}/arm-linux-gnueabihf/sysroot")
endif()
set(CMAKE_SYSROOT ${TARGET_SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${TARGET_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# toolchain
get_filename_component(C_COMPILER "${TOOLCHAIN_PREFIX}gcc" REALPATH BASE_DIR "${TOOLCHAIN_PATH}")
get_filename_component(CXX_COMPILER "${TOOLCHAIN_PREFIX}g++" REALPATH BASE_DIR "${TOOLCHAIN_PATH}")

set(CMAKE_C_COMPILER ${C_COMPILER})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_CXX_COMPILER ${CXX_COMPILER})

if(NOT RPI_ARCH)
    set(RPI_ARCH
      armv8-a		#RPi support: 2B 1.2, 3B, 3B+
      #armv7-a	#RPi support: 2B
      #armv8		#RPi support: 3A+
      #armv6z		#RPi support: 1A, 1A+, 1B, Zero 1.2, Zero 1.3, Zero W
    )
endif()
set(PACKAGE_ARCH ${RPI_ARCH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${RPI_ARCH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${RPI_ARCH}")

set(EXTERNAL_SYSROOT ${CMAKE_SOURCE_DIR}/external)

link_directories(
    ${EXTERNAL_SYSROOT}/lib
    ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader
    )
