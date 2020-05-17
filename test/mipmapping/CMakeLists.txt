file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(mipmapping ${testSrc} )
target_compile_options(mipmapping PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(mipmapping vulkan $<TARGET_OBJECTS:QPUassembler>)
