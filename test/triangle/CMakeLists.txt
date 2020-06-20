file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(triangle ${testSrc} )

target_link_libraries(triangle vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(triangle vulkan-loader QPUassembler)

add_testcase(triangle)
