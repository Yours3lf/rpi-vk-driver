file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(cubeMipmapping ${testSrc} )
target_compile_options(cubeMipmapping PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(cubeMipmapping vulkan $<TARGET_OBJECTS:QPUassembler>)
