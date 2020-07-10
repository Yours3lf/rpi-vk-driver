file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(cubeMipmapping ${testSrc} )

target_link_libraries(cubeMipmapping vulkan $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(cubeMipmapping vulkan-loader QPUassembler)

add_testcase(cubeMipmapping)
