if(NOT VULKAN_VERSION)
    set(VULKAN_VERSION 1.2.141)
endif()

set(VULKAN_HEADERS_TAG sdk-${VULKAN_VERSION})
set(VULKAN_LOADER_TAG sdk-${VULKAN_VERSION})
set(VULKAN_TOOLS_TAG sdk-${VULKAN_VERSION})

option(BUILD_WSI_XCB_SUPPORT "Build Vulkan-Loader with XCB Support" OFF)
option(BUILD_WSI_XLIB_SUPPORT "Build Vulkan-Loader with XLIB Support" OFF)
option(BUILD_WSI_WAYLAND_SUPPORT "Build Vulkan-Loader with Wayland Support" OFF)

# this option requires GCC > 4.9
option(BUILD_VULKAN_TOOLS_INFO "Build Vulkan-Tools vulkaninfo" OFF)
option(BUILD_VULKAN_TOOLS_CUBE "Build Vulkan-Tools cube" OFF)

option(BUILD_INSTALL_TESTS "Install Test cases" OFF)