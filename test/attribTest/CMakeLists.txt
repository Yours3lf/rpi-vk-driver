file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(attribTest ${testSrc} )

target_link_libraries(attribTest vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(attribTest vulkan-loader QPUassembler)

add_testcase(attribTest)
