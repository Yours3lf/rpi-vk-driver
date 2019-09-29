file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(indexedTriangle ${testSrc} )
target_compile_options(indexedTriangle PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(indexedTriangle vulkan)
