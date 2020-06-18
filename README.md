# RPi-VK-Driver
RPi-VK-Driver is a low level GPU driver for the Broadcom Videocore IV GPU that implements a subset of the Vulkan® standard. The implementation is not conformant to the standard (therefore it cannot be called a Vulkan driver, officially) but tries to follow it as closely as the hardware allows for it.<br>
Compared to the available OpenGL drivers it offers superb speed including precise and predictable memory management and multi-threaded command submission. It also offers a wider feature set such as MSAA support, low level assembly shaders and performance counters.
On the other hand it currently does not support GLSL shaders.

## Building
Please follow the instructions in the Build.md file.
https://github.com/Yours3lf/rpi-vk-driver/blob/master/BUILD.md

## Wiki
For further information please take a look at the Wiki section:
https://github.com/Yours3lf/rpi-vk-driver/wiki

## Supported hardware
The driver currently supports the following Raspberry Pi models:
- Zero
- Zero W
- 1 Model A
- 1 Model A+
- 1 Model B
- 1 Model B+
- 2 Model B
- 3 Model A+
- 3 Model B
- 3 Model B+
- Compute Module 1
- Compute Module 3
- Compute Module 3 lite
- Compute Module 3+
- Compute Module 3+ lite

