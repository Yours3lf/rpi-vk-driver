# How to compile on a Raspberry Pi

### Install prerequisites
sudo apt-get install cmake

### Clone RPi VK Driver
git clone https://github.com/Yours3lf/rpi-vk-driver.git  
cd rpi-vk-driver  
mkdir build  
cd build  

### Run CMake
cmake .. -DCMAKE_BUILD_TYPE=Release  

### Build project
cmake --build . --target all  

### Run install.sh
../install.sh

### Install the Vulkan-Loader
Follow the Vulkan-Loader repository instructions to build and install the Vulkan-Loader:
https://github.com/KhronosGroup/Vulkan-Loader/blob/master/BUILD.md

