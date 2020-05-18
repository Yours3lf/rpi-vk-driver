file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(clear ${testSrc})
target_compile_options(clear PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(clear vulkan $<TARGET_OBJECTS:QPUassembler>)
