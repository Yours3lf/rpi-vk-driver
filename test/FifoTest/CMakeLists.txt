file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(FifoTest ${testSrc})

target_link_libraries(FifoTest rpi-vk-driver)
add_dependencies(FifoTest rpi-vk-driver)

add_testcase(FifoTest)
