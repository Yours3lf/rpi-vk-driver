file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(texturing ${testSrc} )
target_compile_options(texturing PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(texturing vulkan $<TARGET_OBJECTS:QPUassembler>)
