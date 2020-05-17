file(GLOB driverSrc
	"*.h"
	"*.c"
)

add_library(rpi-vk-driver SHARED ${driverSrc})
target_compile_options(rpi-vk-driver PRIVATE -Wall -Wextra -Wpacked -Wcast-align -std=c11
	-march=${RPI_ARCH}
	)

target_link_libraries(rpi-vk-driver drm pthread expat z $<TARGET_OBJECTS:brcm> $<TARGET_OBJECTS:QPUassembler>)
