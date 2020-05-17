file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTex ${testSrc} )
target_compile_options(depthTex PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(depthTex vulkan $<TARGET_OBJECTS:QPUassembler>)
