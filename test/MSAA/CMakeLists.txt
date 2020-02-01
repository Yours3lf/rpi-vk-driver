file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(MSAA ${testSrc} )
target_compile_options(MSAA PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(MSAA vulkan)
