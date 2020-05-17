file(GLOB driverSrc
	"*.h"
	"*.c"
)

add_library(rpi-vk-driver SHARED ${driverSrc})
target_compile_options(rpi-vk-driver PRIVATE -Wall -Werror=implicit-function-declaration -std=c11 -Wpacked -Wcast-align)

target_link_libraries(rpi-vk-driver drm pthread expat z $<TARGET_OBJECTS:brcm> $<TARGET_OBJECTS:QPUassembler>)
