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
	VkRpiSurfaceCreateFlagsKHR    flags; //reserved
	//maybe include some other stuff dunno
} VkRpiSurfaceCreateInfoKHR;

//extension name something like: VK_KHR_rpi_surface
//extension that allows developers to create a surface to render to on Raspbian Stretch Lite
VkResult vkCreateRpiSurfaceKHR(
		VkInstance                                  instance,
		const VkRpiSurfaceCreateInfoKHR*            pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSurfaceKHR*                               pSurface);

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
VkResult vkCreateShaderModuleFromRpiAssemblyKHR(
		VkDevice									device,
		uint32_t									numBytes,
		char*										byteStream,
		const VkAllocationCallbacks*				pAllocator,
		VkShaderModule*								pShaderModule
		);


#ifdef __cplusplus
}
#endif
