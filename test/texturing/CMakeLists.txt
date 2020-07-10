file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(texturing ${testSrc} )

target_link_libraries(texturing vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(texturing vulkan-loader QPUassembler)

add_testcase(texturing)
