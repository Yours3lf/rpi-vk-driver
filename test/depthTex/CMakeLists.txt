file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(depthTex ${testSrc} )
target_compile_options(depthTex PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(depthTex vulkan $<TARGET_OBJECTS:QPUassembler>)
