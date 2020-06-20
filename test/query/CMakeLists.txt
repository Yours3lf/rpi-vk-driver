file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(query ${testSrc} )

target_link_libraries(query vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(query vulkan-loader QPUassembler)

add_testcase(query)
