file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(stencilTest ${testSrc} )
target_compile_options(stencilTest PRIVATE -Wall -std=c++11 -std=c11
	-march=${RPI_ARCH}
	)

target_link_libraries(stencilTest vulkan $<TARGET_OBJECTS:QPUassembler>)
