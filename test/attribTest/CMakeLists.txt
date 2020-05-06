file(GLOB testSrc
	"*.h"
	"*.cpp"
)

add_executable(attribTest ${testSrc} )
target_compile_options(attribTest PRIVATE -Wall -std=c++11 -std=c11)

target_link_libraries(attribTest vulkan $<TARGET_OBJECTS:QPUassembler>)
