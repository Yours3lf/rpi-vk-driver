file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(clearTest ${testSrc} )
target_compile_options(clearTest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(clearTest vulkan $<TARGET_OBJECTS:QPUassembler>)
