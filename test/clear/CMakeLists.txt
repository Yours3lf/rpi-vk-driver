file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(clear ${testSrc})
target_compile_options(clear PRIVATE -Wall -std=c++11)

target_link_libraries(clear vulkan-1-rpi)
