file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(CPAtest ${testSrc})

target_link_libraries(CPAtest rpi-vk-driver)
add_dependencies(CPAtest rpi-vk-driver)

add_testcase(CPAtest)
