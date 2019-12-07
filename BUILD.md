# How to cross compile the RPi VK Driver 

### Clone compiler
git clone https://github.com/raspberrypi/tools.git

### Establish sysroot
rsync -az --delete-after --safe-links [user]@[ipaddress]:/{lib,usr} tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot

### Clone RPi VK Driver
git clone https://github.com/Yours3lf/rpi-vk-driver.git  
cd rpi-vk-driver  
mkdir build  
cd build  

### Run CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_BUILD_TYPE=Release  

### Build project
cmake --build . --target all  

### Deploy files
scp librpi-vk-driver.so [user]@[ipaddress]:/home/[user]/librpi-vk-driver.so  
scp rpi-vk-driver.json [user]@[ipaddress]:/home/[user]/rpi-vk-driver.json  
scp install.sh [user]@[ipaddress]:/home/[user]/install.sh  

### Run install.sh on your RPi
