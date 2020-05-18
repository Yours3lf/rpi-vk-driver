file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(query ${testSrc} )
target_compile_options(query PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(query vulkan $<TARGET_OBJECTS:QPUassembler>)
