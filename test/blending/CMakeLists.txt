file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(blending ${testSrc} )

target_link_libraries(blending vulkan  $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(blending vulkan-loader QPUassembler)

add_testcase(blending)
