
# How to compile on a Raspberry Pi

### Install prerequisites

    sudo apt-get install cmake
    sudo apt remove libvulkan1 vulkan-headers

The Vulkan-Loader version distributed with Raspbian (1.1.97-2) does not support the required performance counters.

### Clone, build, and install RPi VK Driver

    git clone https://github.com/Yours3lf/rpi-vk-driver.git
    cd rpi-vk-driver
    mkdir build && cd build
    cmake ..
    make
    sudo make install

# How to run tests on a Raspberry Pi

### Prerequisites

* Enable Full Desktop KDM using `sudo raspi-config` under Advanced if you haven't already
* Reboot into CLI using `sudo raspi-config`

### Clone, build, and run tests

    git clone https://github.com/Yours3lf/rpi-vk-driver.git
    cd rpi-vk-driver
    mkdir build && cd build
    cmake ..
    make install
    make test

### Gotchas
* You cannot run tests from the desktop environment as there can only be one DRM-Master.  Tests can only be run after rebooting into CLI.

# How to cross-compile on Linux desktop

    git clone https://github.com/Yours3lf/rpi-vk-driver.git
    cd rpi-vk-driver
    git clone https://github.com/raspberrypi/tools.git
    export PATH=`pwd`/tools/arm-bcm2708/arm-linux-gnueabihf/bin:$PATH
    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../gcc.toolchain.cmake -DCMAKE_STAGING_PREFIX=./out/usr/local
    make install -j

# How to cross-compile on Linux desktop for specific device

    git clone https://github.com/Yours3lf/rpi-vk-driver.git
    cd rpi-vk-driver
    git clone https://github.com/raspberrypi/tools.git
    export PATH=`pwd`/tools/arm-bcm2708/arm-linux-gnueabihf/bin:$PATH
    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../gcc.toolchain.cmake -DCMAKE_STAGING_PREFIX=./out/usr/local -DRPI_ARCH=armv8-a
    make -j
    make package
    cmake .. -DRPI_ARCH=armv7-a
    make -j
    make package
    cmake .. -DRPI_ARCH=armv6z
    make -j
    make package

# How to generate Debian package

    git clone https://github.com/Yours3lf/rpi-vk-driver.git
    cd rpi-vk-driver
    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../gcc.toolchain.cmake
    make package -j

# How to check contents of Debian package

    dpkg -c rpi-vulkan-driver-1.0.0-1.2.141.Linux-arm.deb

# Using gcc.toolchain.cmake

In order to use this toolchain file, it is passed in to CMake as a variable.

    cmake .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../gcc.toolchain.cmake

If you pass variables before the toolchain, they will be available in the toolchain file.  Here the updated TARGET_SYSROOT value is available for use in the toolchain file.

    cmake .. -DTARGET_SYSROOT=/mnt/rootfs -DCMAKE_TOOLCHAIN_FILE=`pwd`/../gcc.toolchain.cmake

## TOOLCHAIN_TRIPLE
The toolchain will use this value to determine the toolchain path.  If you are using the default `arm-linux-gnueabihf` value, the bin folder of this toolchain needs to be added to your path.

## TARGET_SYSROOT
If using the default Raspberry Pi toolchain it will use the sysroot from this toolchain.  If you use a different toolchain, you will need to update this variable.

`-DSYSROOT=/mnt/rootfs`

## RPI_ARCH

`RPI_ARCH` | RPi
---|:---:
`armv8-a` | 2B 1.2, 3B, 3B+
`armv7-a` | 2B
`armv8` | 3A+
`armv6z` | 1A, 1A+, 1B, Zero 1.2, Zero 1.3, Zero W

The default value is `armv8-a`

# CMAKE Variables

### BUILD_NUMBER
For CI system.  Default is `1.0.0`

### CMAKE_BUILD_TYPE

Sets the build type.  Options are `Debug`, `Release`, or `MinSizeRel`.  Default is `Release`

### TARGET_SYSROOT

Point to the target sysroot.

### CMAKE_STAGING_PREFIX

The sysroot staging path.  See Linux Cross-compile example above for usage.  Default value is ""

### CMAKE_INSTALL_PREFIX

This is what the prefix should be when installed on the target sysroot.  The default for this value on Linux is `/usr/local`.

### BUILD_TESTING

Enables building Unit Test Cases.  If Cross-compiling, `make test` will not run any tests.  Default is `ON`

### VULKAN_VERSION

Selects the sdk version for Vulkan-Headers, and Vulkan-Loader.  Default branch is `sdk-1.2.141`

### BUILD_WSI_XCB_SUPPORT

Builds the Vulkan-Loader with XCB Support.  Default is `OFF`

### BUILD_WSI_XLIB_SUPPORT

Builds the Vulkan-Loader with XLIB Support.  Default is `OFF`

### BUILD_WSI_WAYLAND_SUPPORT

Builds the Vulkan-Loader with Wayland Support.  Default is `OFF`
