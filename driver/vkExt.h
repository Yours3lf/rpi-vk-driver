#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//we need something like the other platforms to create surfaces on the RPI
//so I created this little "extension"
//full spec in this file ;)

typedef enum VkRpiSurfaceCreateFlagsKHR {
	//reserved
	VK_RPI_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkRpiSurfaceCreateFlagsKHR;

typedef struct VkRpiSurfaceCreateInfoKHR {
			   VkStructureType               sType;
			   const void*                   pNext;
			   VkRpiSurfaceCreateFlagsKHR    flags;
			   //maybe include some other stuff dunno
		   } VkRpiSurfaceCreateInfoKHR;

//extension name something like: VK_KHR_rpi_surface
VkResult vkCreateRpiSurfaceKHR(
			  VkInstance                                  instance,
			  const VkRpiSurfaceCreateInfoKHR*            pCreateInfo,
			  const VkAllocationCallbacks*                pAllocator,
			  VkSurfaceKHR*                               pSurface);


#ifdef __cplusplus
}
#endif
