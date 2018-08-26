file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(triangle ${testSrc})
target_compile_options(triangle PRIVATE -Wall -std=c++11)

target_link_libraries(triangle vulkan-1-rpi)
