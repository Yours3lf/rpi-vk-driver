
include(ExternalProject)

ExternalProject_Add(vulkan-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG ${VULKAN_HEADERS_TAG}
    GIT_SHALLOW true
    BUILD_IN_SOURCE 0
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DTARGET_SYSROOT=${TARGET_SYSROOT}
        -DTOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_STAGING_PREFIX=${CMAKE_SOURCE_DIR}/external
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_VERBOSE_MAKEFILE=TRUE
)

ExternalProject_Add(vulkan-loader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG ${VULKAN_LOADER_TAG}
    GIT_SHALLOW true
    BUILD_IN_SOURCE 0
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DTARGET_SYSROOT=${TARGET_SYSROOT}
        -DTOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_STAGING_PREFIX=${CMAKE_STAGING_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_VERBOSE_MAKEFILE=TRUE
        -DVULKAN_HEADERS_INSTALL_DIR=${CMAKE_SOURCE_DIR}/external
        -DBUILD_WSI_XCB_SUPPORT=${BUILD_WSI_XCB_SUPPORT}
        -DBUILD_WSI_XLIB_SUPPORT=${BUILD_WSI_XLIB_SUPPORT}
        -DBUILD_WSI_WAYLAND_SUPPORT=${BUILD_WSI_WAYLAND_SUPPORT}
    INSTALL_COMMAND ""
)
add_dependencies(vulkan-loader vulkan-headers)

if(BUILD_VULKAN_TOOLS_INFO OR BUILD_VULKAN_TOOLS_CUBE)
    ExternalProject_Add(vulkan-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Tools.git
        GIT_TAG ${VULKAN_TOOLS_TAG}
        GIT_SHALLOW true
        BUILD_IN_SOURCE 0
        UPDATE_COMMAND ""
        CMAKE_ARGS
            -DTARGET_SYSROOT=${TARGET_SYSROOT}
            -DTOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_STAGING_PREFIX=${CMAKE_STAGING_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_VERBOSE_MAKEFILE=TRUE
            -DVulkanRegistry_DIR=${CMAKE_SOURCE_DIR}/external/share/vulkan/registry
            -DVulkanHeaders_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/external/include
            -DVulkan_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/external/include
            -DVulkan_LIBRARY=${CMAKE_STAGING_PREFIX}/lib/libvulkan.so
            -DBUILD_ICD=OFF
            -DINSTALL_ICD=OFF
            -DBUILD_CUBE=${BUILD_VULKAN_TOOLS_CUBE}
            -DBUILD_VULKANINFO=${BUILD_VULKAN_TOOLS_INFO}
            -DBUILD_WSI_XCB_SUPPORT=${BUILD_WSI_XCB_SUPPORT}
            -DBUILD_WSI_XLIB_SUPPORT=${BUILD_WSI_XLIB_SUPPORT}
            -DBUILD_WSI_WAYLAND_SUPPORT=${BUILD_WSI_WAYLAND_SUPPORT}
        INSTALL_COMMAND ""
    )
    add_dependencies(vulkan-tools vulkan-loader)
    if(NOT CMAKE_CROSSCOMPILING)
        add_test(NAME vulkaninfo COMMAND ${CMAKE_BINARY_DIR}/vulkan-tools-prefix/src/vulkan-tools-build/vulkaninfo/vulkaninfo)
    endif()
endif()
