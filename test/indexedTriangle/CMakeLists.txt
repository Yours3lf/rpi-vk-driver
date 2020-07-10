file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(indexedTriangle ${testSrc} )

target_link_libraries(indexedTriangle vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(indexedTriangle vulkan-loader QPUassembler)

add_testcase(indexedTriangle)
