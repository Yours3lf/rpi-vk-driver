file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(multithreading ${testSrc} )
target_compile_options(multithreading PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(multithreading vulkan $<TARGET_OBJECTS:QPUassembler>)
