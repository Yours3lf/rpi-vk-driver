file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(HDR ${testSrc} )
target_compile_options(HDR PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH}
	)

target_link_libraries(HDR vulkan $<TARGET_OBJECTS:QPUassembler>)
