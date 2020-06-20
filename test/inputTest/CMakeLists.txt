file(GLOB testSrc
	"*.h"
	"*.cpp"
	"../inputHandler/inputHandler.c"
)

add_executable(inputTest ${testSrc})

target_link_libraries(inputTest mtdev evdev udev input)

