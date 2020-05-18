file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(mintest ${testSrc})
target_compile_options(mintest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(mintest vulkan $<TARGET_OBJECTS:QPUassembler>)
