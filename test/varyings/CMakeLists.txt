file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(varyings ${testSrc} )
target_compile_options(varyings PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(varyings vulkan-1-rpi)
