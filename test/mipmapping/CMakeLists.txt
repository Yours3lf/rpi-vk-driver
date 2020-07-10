file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(mipmapping ${testSrc} )

target_link_libraries(mipmapping vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(mipmapping vulkan-loader QPUassembler)

add_testcase(mipmapping)
