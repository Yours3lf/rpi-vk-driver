file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTest ${testSrc} )

target_link_libraries(depthTest vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(depthTest vulkan-loader QPUassembler)

add_testcase(depthTest)
