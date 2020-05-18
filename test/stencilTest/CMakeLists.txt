file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(stencilTest ${testSrc} )
target_compile_options(stencilTest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(stencilTest vulkan $<TARGET_OBJECTS:QPUassembler>)
