file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(mipmapping ${testSrc} )
target_compile_options(mipmapping PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(mipmapping vulkan)
