file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(cubemapping ${testSrc} )

target_link_libraries(cubemapping vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(cubemapping vulkan-loader QPUassembler)

add_testcase(cubemapping)
