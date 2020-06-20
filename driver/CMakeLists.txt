file(GLOB driverSrc
    "*.h"
    "*.c"
)

add_library(rpi-vk-driver SHARED ${driverSrc})
target_compile_options(rpi-vk-driver PRIVATE -Wextra -Wpacked -Wcast-align)
target_link_libraries(rpi-vk-driver drm pthread expat z dl $<TARGET_OBJECTS:brcm> $<TARGET_OBJECTS:QPUassembler>)
set_target_properties(rpi-vk-driver PROPERTIES C_STANDARD 11 C_EXTENSIONS OFF)
add_dependencies(rpi-vk-driver vulkan-headers brcm QPUassembler)
install(TARGETS rpi-vk-driver LIBRARY DESTINATION lib)

configure_file(rpi-vk-driver.json.in ${CMAKE_BINARY_DIR}/rpi-vk-driver.json @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/rpi-vk-driver.json DESTINATION share/vulkan/icd.d)

if(NOT CMAKE_CROSSCOMPILING)
    # prevents sudo make
    install(FILES
        ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader/libvulkan.so
        ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader/libvulkan.so.1
        ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader/libvulkan.so.${VULKAN_VERSION}
        DESTINATION lib
    )
    install(FILES
        ${CMAKE_BINARY_DIR}/vulkan-loader-prefix/src/vulkan-loader-build/loader/vulkan.pc
        DESTINATION lib/pkgconfig
    )
endif()
