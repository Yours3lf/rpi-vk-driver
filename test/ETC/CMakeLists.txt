file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(ETC ${testSrc} )
target_compile_options(ETC PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(ETC vulkan $<TARGET_OBJECTS:QPUassembler>)
