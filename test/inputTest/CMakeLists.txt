file(GLOB testSrc
	"*.h"
	"*.cpp"
	"../inputHandler/inputHandler.c"
)

add_executable(inputTest ${testSrc})
target_compile_options(inputTest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(inputTest mtdev evdev udev input)

