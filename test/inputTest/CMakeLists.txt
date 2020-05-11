file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(inputTest ${testSrc})
target_compile_options(inputTest PRIVATE -Wall -std=c++11)

target_link_libraries(inputTest mtdev evdev udev input)

