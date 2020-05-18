file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(varyings ${testSrc} )
target_compile_options(varyings PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(varyings vulkan $<TARGET_OBJECTS:QPUassembler>)
