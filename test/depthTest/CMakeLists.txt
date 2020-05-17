file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTest ${testSrc} )
target_compile_options(depthTest PRIVATE -Wall -std=c++11 -std=c11
	-march=${RPI_ARCH}
	)

target_link_libraries(depthTest vulkan $<TARGET_OBJECTS:QPUassembler>)
