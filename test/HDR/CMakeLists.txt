file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(HDR ${testSrc} )
target_compile_options(HDR PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(HDR vulkan)
