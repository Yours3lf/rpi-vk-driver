file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(triangle ${testSrc} )
target_compile_options(triangle PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(triangle vulkan $<TARGET_OBJECTS:QPUassembler>)
