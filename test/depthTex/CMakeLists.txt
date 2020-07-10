file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTex ${testSrc} )

target_link_libraries(depthTex vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(depthTex vulkan-loader QPUassembler)

add_testcase(depthTex)
