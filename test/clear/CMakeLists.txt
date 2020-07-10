file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(clear ${testSrc})

target_link_libraries(clear vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(clear vulkan-loader QPUassembler)

add_testcase(clear)
