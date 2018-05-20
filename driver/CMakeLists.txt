file(GLOB driverSrc
	"*.h"
	"*.c"
)

add_library(vulkan-1-rpi SHARED ${driverSrc})
target_compile_options(vulkan-1-rpi PRIVATE -Wall -std=c99)

target_link_libraries(vulkan-1-rpi drm pthread)
