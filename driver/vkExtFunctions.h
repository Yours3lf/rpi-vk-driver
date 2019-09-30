#pragma once

#include "vkExt.h"

#ifdef __cplusplus
extern "C" {
#endif

//extension name something like: VK_KHR_rpi_surface
//extension that allows developers to create a surface to render to on Raspbian Stretch Lite
extern VkResult rpi_vkCreateRpiSurfaceEXT(
		VkInstance		                            instance,
		const VkRpiSurfaceCreateInfoEXT*            pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSurfaceKHR*                               pSurface);

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
extern VkResult rpi_vkCreateShaderModuleFromRpiAssemblyEXT(
		VkDevice                            device,
		VkRpiShaderModuleAssemblyCreateInfoEXT*		pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkShaderModule*								pShaderModule
		);

extern void* _getFuncPtr(const char* name);

static VkResult rpi_vkGetRpiExtensionPointerEXT(
		VkPhysicalDevice                            physicalDevice
		)
{
	//TODO how do we handle our "custom" extensions towards the loader????

	uint32_t ret = 0;//(uint32_t)_getFuncPtr((const char*)physicalDevice);

	if(ret)
	{
		return (VkResult)ret;
	}

	return VK_SUCCESS;
}

//TODO performance counters / perfmon


#ifdef __cplusplus
}
#endif
