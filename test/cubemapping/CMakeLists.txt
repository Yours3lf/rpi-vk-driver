file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(cubemapping ${testSrc} )
target_compile_options(cubemapping PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(cubemapping vulkan $<TARGET_OBJECTS:QPUassembler>)
