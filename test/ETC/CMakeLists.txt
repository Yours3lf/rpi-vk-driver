file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(ETC ${testSrc} )
target_compile_options(ETC PRIVATE -Wall -std=c++11 -std=c11
	-march=${RPI_ARCH}
	)

target_link_libraries(ETC vulkan $<TARGET_OBJECTS:QPUassembler>)
