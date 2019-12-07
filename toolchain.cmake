set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

get_filename_component(C_COMPILER "../../tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
get_filename_component(CXX_COMPILER "../../tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
get_filename_component(SYSROOT "../../sysroot" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_C_COMPILER ${C_COMPILER})
set(CMAKE_CXX_COMPILER ${CXX_COMPILER})
set(CMAKE_SYSROOT ${SYSROOT})