file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(HDR ${testSrc} )

target_link_libraries(HDR vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(HDR vulkan-loader QPUassembler)

add_testcase(HDR)
