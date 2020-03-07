file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(CPAtest ${testSrc})
target_compile_options(CPAtest PRIVATE -Wall -std=c++11)

target_link_libraries(CPAtest rpi-vk-driver)
