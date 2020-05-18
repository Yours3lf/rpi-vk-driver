file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(CPAtest ${testSrc})
target_compile_options(CPAtest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(CPAtest rpi-vk-driver)
