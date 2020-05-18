file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTest ${testSrc} )
target_compile_options(depthTest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(depthTest vulkan $<TARGET_OBJECTS:QPUassembler>)
