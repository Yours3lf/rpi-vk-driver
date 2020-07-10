file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(stencilTest ${testSrc} )

target_link_libraries(stencilTest vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(stencilTest vulkan-loader QPUassembler)

add_testcase(stencilTest)
