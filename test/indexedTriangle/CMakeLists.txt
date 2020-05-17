file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(indexedTriangle ${testSrc} )
target_compile_options(indexedTriangle PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(indexedTriangle vulkan $<TARGET_OBJECTS:QPUassembler>)
