file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(cubemapping ${testSrc} )
target_compile_options(cubemapping PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(cubemapping vulkan)
