file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(MSAA ${testSrc} )
target_compile_options(MSAA PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(MSAA vulkan $<TARGET_OBJECTS:QPUassembler>)
