file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(FifoTest ${testSrc})
target_compile_options(FifoTest PRIVATE -Wall -std=c++11
	-march=${RPI_ARCH} -fPIC
	)

target_link_libraries(FifoTest rpi-vk-driver)
