file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(multithreading ${testSrc} )

target_link_libraries(multithreading vulkan pthread $<TARGET_OBJECTS:QPUassembler>)
add_dependencies(multithreading vulkan-loader QPUassembler)

add_testcase(multithreading)
