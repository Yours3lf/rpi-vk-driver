#pragma once

#include "vkExt.h"

#ifdef __cplusplus
extern "C" {
#endif

//extension name something like: VK_KHR_rpi_surface
//extension that allows developers to create a surface to render to on Raspbian Stretch Lite
extern VkResult rpi_vkCreateRpiSurfaceEXT(
		VkPhysicalDevice		                    physicalDevice);

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
extern VkResult rpi_vkCreateShaderModuleFromRpiAssemblyEXT(
		VkPhysicalDevice		                    physicalDevice);

//TODO performance counters / perfmon


#ifdef __cplusplus
}
#endif
