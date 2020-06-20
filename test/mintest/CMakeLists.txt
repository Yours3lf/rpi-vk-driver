file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(mintest ${testSrc})

target_link_libraries(mintest vulkan)
add_dependencies(mintest vulkan-loader)

add_testcase(mintest)
