file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(clearTest ${testSrc} )

target_link_libraries(clearTest vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(clearTest vulkan-loader QPUassembler)

add_testcase(clearTest)
