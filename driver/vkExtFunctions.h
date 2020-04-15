#pragma once

#include "vkExt.h"

#ifdef __cplusplus
extern "C" {
#endif

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
extern VkResult rpi_vkCreateShaderModuleFromRpiAssemblyEXT(
		VkPhysicalDevice		                    physicalDevice);

//TODO performance counters / perfmon


#ifdef __cplusplus
}
#endif
