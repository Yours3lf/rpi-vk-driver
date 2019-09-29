file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(texturing ${testSrc} )
target_compile_options(texturing PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(texturing vulkan)
