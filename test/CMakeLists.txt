file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(vulkanTest ${testSrc})
target_compile_options(vulkanTest PRIVATE -Wall -std=c++11)

target_link_libraries(vulkanTest vulkan-1-rpi)
