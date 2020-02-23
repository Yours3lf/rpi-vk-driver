file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(query ${testSrc} )
target_compile_options(query PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(query vulkan)
