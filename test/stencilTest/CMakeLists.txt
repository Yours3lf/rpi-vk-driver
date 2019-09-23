file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(stencilTest ${testSrc} )
target_compile_options(stencilTest PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(stencilTest vulkan-1-rpi)
