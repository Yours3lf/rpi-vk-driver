#include "common.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#devsandqueues-physical-device-enumeration
 * If pPhysicalDevices is NULL, then the number of physical devices available is returned in pPhysicalDeviceCount. Otherwise, pPhysicalDeviceCount must point to a
 * variable set by the user to the number of elements in the pPhysicalDevices array, and on return the variable is overwritten with the number of handles actually
 * written to pPhysicalDevices. If pPhysicalDeviceCount is less than the number of physical devices available, at most pPhysicalDeviceCount structures will be written.
 * If pPhysicalDeviceCount is smaller than the number of physical devices available, VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the
 * available physical devices were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumeratePhysicalDevices(
		VkInstance                                  instance,
		uint32_t*                                   pPhysicalDeviceCount,
		VkPhysicalDevice*                           pPhysicalDevices)
{
	assert(instance);

	int numGPUs = 1;

	assert(pPhysicalDeviceCount);

	if(!pPhysicalDevices)
	{
		*pPhysicalDeviceCount = numGPUs;
		return VK_SUCCESS;
	}

	int arraySize = *pPhysicalDeviceCount;
	int elementsWritten = min(numGPUs, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pPhysicalDevices[c] = &instance->dev;
	}

	*pPhysicalDeviceCount = elementsWritten;

	if(arraySize < numGPUs)
	{
		return VK_INCOMPLETE;
	}
	else
	{
		return VK_SUCCESS;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceProperties
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceProperties(
		VkPhysicalDevice                            physicalDevice,
		VkPhysicalDeviceProperties*                 pProperties)
{
	assert(physicalDevice);
	assert(pProperties);

	VkPhysicalDeviceSparseProperties sparseProps =
	{
		.residencyStandard2DBlockShape = 1,
		.residencyStandard2DMultisampleBlockShape = 1,
		.residencyStandard3DBlockShape = 1,
		.residencyAlignedMipSize = 1,
		.residencyNonResidentStrict = 1
	};

	pProperties->apiVersion = VK_DRIVER_VERSION;
	pProperties->driverVersion = 1; //we'll simply call this v1
	pProperties->vendorID = 0x14E4; //Broadcom
	pProperties->deviceID = 0x5250; //RP in HEX
	pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
	strcpy(pProperties->deviceName, "VideoCore IV HW");
	//TODO pProperties->pipelineCacheUUID
	pProperties->limits = _limits;
	pProperties->sparseProperties = sparseProps;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceFeatures
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFeatures(
		VkPhysicalDevice                            physicalDevice,
		VkPhysicalDeviceFeatures*                   pFeatures)
{
	assert(physicalDevice);
	assert(pFeatures);

	*pFeatures = _features;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateDeviceExtensionProperties
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateDeviceExtensionProperties(
		VkPhysicalDevice                            physicalDevice,
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(physicalDevice);
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numDeviceExtensions;
		return VK_SUCCESS;
	}

	//TODO layers

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numDeviceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = deviceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	if(arraySize < numDeviceExtensions)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceQueueFamilyProperties
 * If pQueueFamilyProperties is NULL, then the number of queue families available is returned in pQueueFamilyPropertyCount.
 * Otherwise, pQueueFamilyPropertyCount must point to a variable set by the user to the number of elements in the pQueueFamilyProperties array,
 * and on return the variable is overwritten with the number of structures actually written to pQueueFamilyProperties. If pQueueFamilyPropertyCount
 * is less than the number of queue families available, at most pQueueFamilyPropertyCount structures will be written.
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceQueueFamilyProperties(
		VkPhysicalDevice                            physicalDevice,
		uint32_t*                                   pQueueFamilyPropertyCount,
		VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
	assert(physicalDevice);
	assert(pQueueFamilyPropertyCount);

	if(!pQueueFamilyProperties)
	{
		*pQueueFamilyPropertyCount = 1;
		return;
	}

	int arraySize = *pQueueFamilyPropertyCount;
	int elementsWritten = min(numQueueFamilies, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pQueueFamilyProperties[c] = _queueFamilyProperties[c];
	}

	*pQueueFamilyPropertyCount = elementsWritten;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateDevice
 * vkCreateDevice verifies that extensions and features requested in the ppEnabledExtensionNames and pEnabledFeatures
 * members of pCreateInfo, respectively, are supported by the implementation. If any requested extension is not supported,
 * vkCreateDevice must return VK_ERROR_EXTENSION_NOT_PRESENT. If any requested feature is not supported, vkCreateDevice must return
 * VK_ERROR_FEATURE_NOT_PRESENT. Support for extensions can be checked before creating a device by querying vkEnumerateDeviceExtensionProperties
 * After verifying and enabling the extensions the VkDevice object is created and returned to the application.
 * If a requested extension is only supported by a layer, both the layer and the extension need to be specified at vkCreateInstance
 * time for the creation to succeed. Multiple logical devices can be created from the same physical device. Logical device creation may
 * fail due to lack of device-specific resources (in addition to the other errors). If that occurs, vkCreateDevice will return VK_ERROR_TOO_MANY_OBJECTS.
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDevice(
		VkPhysicalDevice                            physicalDevice,
		const VkDeviceCreateInfo*                   pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkDevice*                                   pDevice)
{
	assert(physicalDevice);
	assert(pDevice);
	assert(pCreateInfo);

	//TODO store enabled features and extensions
	//and check later on if they are enabled.

	//check for enabled extensions
	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findDeviceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres == -1)
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//check for enabled features
	VkBool32* requestedFeatures = pCreateInfo->pEnabledFeatures;
	VkBool32* supportedFeatures = &_features;

	if(requestedFeatures)
	{
		for(int c = 0; c < numFeatures; ++c)
		{
			if(requestedFeatures[c] && !supportedFeatures[c])
			{
				return VK_ERROR_FEATURE_NOT_PRESENT;
			}
		}
	}

	*pDevice = ALLOCATE(sizeof(_device), 1, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
	if(!*pDevice)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	set_loader_magic_value(&(*pDevice)->loaderData);

	(*pDevice)->dev = physicalDevice;

	(*pDevice)->numEnabledExtensions = 0;

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findDeviceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pDevice)->enabledExtensions[(*pDevice)->numEnabledExtensions] = findres;
			(*pDevice)->numEnabledExtensions++;
		}
	}

	if(requestedFeatures)
	{
		for(int c = 0; c < numFeatures; ++c)
		{
			if(requestedFeatures[c] && !supportedFeatures[c])
			{
				return VK_ERROR_FEATURE_NOT_PRESENT;
			}
		}

		(*pDevice)->enabledFeatures = *pCreateInfo->pEnabledFeatures;
	}
	else
	{
		memset(&(*pDevice)->enabledFeatures, 0, sizeof((*pDevice)->enabledFeatures)); //just disable everything
	}

	//layers ignored per spec
	//pCreateInfo->enabledLayerCount

	for(int c = 0; c < numQueueFamilies; ++c)
	{
		(*pDevice)->queues[c] = 0;
	}

	if(pCreateInfo->queueCreateInfoCount > 0)
	{
		for(int c = 0; c < pCreateInfo->queueCreateInfoCount; ++c)
		{
			(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex] = ALLOCATE(sizeof(_queue)*pCreateInfo->pQueueCreateInfos[c].queueCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

			if(!(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			for(int d = 0; d < pCreateInfo->pQueueCreateInfos[c].queueCount; ++d)
			{
				(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex][d].lastEmitSeqno = 0;
				(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex][d].dev = *pDevice;
				set_loader_magic_value(&(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex][d].loaderData);
			}

			(*pDevice)->numQueues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex] = pCreateInfo->pQueueCreateInfos[c].queueCount;
		}
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetDeviceQueue
 * vkGetDeviceQueue must only be used to get queues that were created with the flags parameter of VkDeviceQueueCreateInfo set to zero.
 * To get queues that were created with a non-zero flags parameter use vkGetDeviceQueue2.
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceQueue(
		VkDevice                                    device,
		uint32_t                                    queueFamilyIndex,
		uint32_t                                    queueIndex,
		VkQueue*                                    pQueue)
{
	assert(device);
	assert(pQueue);

	assert(queueFamilyIndex < numQueueFamilies);
	assert(queueIndex < device->numQueues[queueFamilyIndex]);

	*pQueue = &device->queues[queueFamilyIndex][queueIndex];
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceQueue2(
	VkDevice                                    device,
	const VkDeviceQueueInfo2*                   pQueueInfo,
	VkQueue*                                    pQueue)
{
	assert(device);
	assert(pQueueInfo);
	assert(pQueue);

	//TODO handle pNext

	vkGetDeviceQueue(device, pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueue);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyDevice
 * To ensure that no work is active on the device, vkDeviceWaitIdle can be used to gate the destruction of the device.
 * Prior to destroying a device, an application is responsible for destroying/freeing any Vulkan objects that were created using that device as the
 * first parameter of the corresponding vkCreate* or vkAllocate* command
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyDevice(
		VkDevice                                    device,
		const VkAllocationCallbacks*                pAllocator)
{
	_device* dev = device;

	if(dev)
	{
		for(int c = 0; c < numQueueFamilies; ++c)
		{
			for(int d = 0; d < dev->numQueues[c]; ++d)
			{
				FREE(dev->queues[d]);
			}
		}
		FREE(dev);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumeratePhysicalDeviceGroups
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumeratePhysicalDeviceGroups(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceGroupCount,
	VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
	assert(instance);
	assert(pPhysicalDeviceGroupCount);

	if(!pPhysicalDeviceGroupProperties)
	{
		*pPhysicalDeviceGroupCount = 1;
		return VK_SUCCESS;
	}

	//we don't have any other devices...
	assert(*pPhysicalDeviceGroupCount == 1);

	uint32_t c = 0;
	for(; c < *pPhysicalDeviceGroupCount; ++c)
	{
		pPhysicalDeviceGroupProperties[c].physicalDeviceCount = 1;
		pPhysicalDeviceGroupProperties[c].physicalDevices[0] = &instance->dev;
		pPhysicalDeviceGroupProperties[c].subsetAllocation = 0;
	}

	if(c < 1)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

extern VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL rpi_vkGetInstanceProcAddr(VkInstance                                  instance,
																		  const char*                                 pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL rpi_vkGetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName)
{
	if(
	!strcmp("vkDestroyInstance", pName) ||
	!strcmp("vkEnumeratePhysicalDevices", pName) ||
	!strcmp("vkGetPhysicalDeviceFeatures", pName) ||
	!strcmp("vkGetPhysicalDeviceFormatProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceImageFormatProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceQueueFamilyProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceMemoryProperties", pName) ||
	!strcmp("vkCreateDevice", pName) ||
	!strcmp("vkEnumerateDeviceExtensionProperties", pName) ||
	!strcmp("vkEnumerateDeviceLayerProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceSparseImageFormatProperties", pName) ||
	!strcmp("vkEnumeratePhysicalDeviceGroups", pName) ||
	!strcmp("vkGetPhysicalDeviceFeatures2", pName) ||
	!strcmp("vkGetPhysicalDeviceProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceImageFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceQueueFamilyProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceMemoryProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceSparseImageFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalBufferProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalFenceProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalSemaphoreProperties", pName)
	)
	{
		return 0;
	}


	//there can't be any other device, so this will do fine...
	_device* d = device;
	return rpi_vkGetInstanceProcAddr(d->dev->instance, pName);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties2*                pProperties)
{
	assert(physicalDevice);
	assert(pProperties);
	vkGetPhysicalDeviceProperties(physicalDevice, &pProperties->properties);

	if(pProperties->pNext)
	{
		VkPhysicalDeviceDriverPropertiesKHR* ptr = pProperties->pNext;
		if(ptr->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR)
		{
			ptr->driverID = 0x525049564b; //RPIVK in hex
			const char* driverName = "RPi VK";
			const char* driverInfo = ""; //TODO maybe version number, git info?
			strcpy(ptr->driverName, driverName);
			strcpy(ptr->driverInfo, driverInfo);
			//TODO this is what we are aspiring to pass...
			ptr->conformanceVersion.major = 1;
			ptr->conformanceVersion.minor = 1;
			ptr->conformanceVersion.subminor = 2;
			ptr->conformanceVersion.patch = 1;
		}
	}
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties*                         pFormatProperties)
{
	assert(physicalDevice);
	assert(pFormatProperties);

	//TODO set this per format!
	if(isDepthStencilFormat(format) && format != VK_FORMAT_S8_UINT)
	{
		pFormatProperties->linearTilingFeatures = 0
												| VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												| VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
												;
		pFormatProperties->optimalTilingFeatures = 0
												| VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
												| VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
												| VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												| VK_FORMAT_FEATURE_BLIT_DST_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												| VK_FORMAT_FEATURE_TRANSFER_DST_BIT
												| VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT

												;
		pFormatProperties->bufferFeatures = 0
												| VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												;
	}
	else
	{
		pFormatProperties->linearTilingFeatures = 0
												| VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												;
		pFormatProperties->optimalTilingFeatures = 0
												| VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
												| VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
												| VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
												| VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT
												| VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												| VK_FORMAT_FEATURE_BLIT_DST_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												| VK_FORMAT_FEATURE_TRANSFER_DST_BIT
												;
		pFormatProperties->bufferFeatures = 0
												| VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
												| VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
												| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
												| VK_FORMAT_FEATURE_BLIT_SRC_BIT
												;
	}
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties2*                        pFormatProperties)
{
	assert(physicalDevice);
	assert(pFormatProperties);
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &pFormatProperties->formatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkImageTiling                               tiling,
	VkImageUsageFlags                           usage,
	VkImageCreateFlags                          flags,
	VkImageFormatProperties*                    pImageFormatProperties)
{
	assert(physicalDevice);
	assert(pImageFormatProperties);

	//TODO usage, flags tiling etc.
	//do all this per format...


	VkFormat ycbcrConversionRequiredFormats[] =
	{
	VK_FORMAT_G8B8G8R8_422_UNORM
	,VK_FORMAT_B8G8R8G8_422_UNORM
	,VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
	,VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
	,VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM
	,VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
	,VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM
	,VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
	,VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16
	,VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
	,VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16
	,VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16
	,VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16
	,VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16
	,VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16
	,VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16
	,VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16
	,VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
	,VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16
	,VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16
	,VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16
	,VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16
	,VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16
	,VK_FORMAT_G16B16G16R16_422_UNORM
	,VK_FORMAT_B16G16R16G16_422_UNORM
	,VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM
	,VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
	,VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM
	,VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
	,VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM
	};
	#define numYcbcrConversionRequiredFormats (sizeof(ycbcrConversionRequiredFormats)/sizeof(VkFormat))

	uint32_t supported = 0;
	for(uint32_t c = 0; c < numSupportedFormats; ++c)
	{
		if(format == supportedFormats[c])
		{
			supported = 1;
			break;
		}
	}

	if(!supported)
	{
		return VK_ERROR_FORMAT_NOT_SUPPORTED;
	}

	pImageFormatProperties->maxArrayLayers = _limits.maxImageArrayLayers;

	pImageFormatProperties->maxExtent.width = 1;
	pImageFormatProperties->maxExtent.height = 1;
	pImageFormatProperties->maxExtent.depth = 1;

	pImageFormatProperties->sampleCounts = _limits.framebufferColorSampleCounts;

	if(type == VK_IMAGE_TYPE_1D)
	{
		pImageFormatProperties->maxExtent.width = _limits.maxImageDimension1D;
		pImageFormatProperties->maxMipLevels = ulog2(_limits.maxImageDimension1D) + 1;
	}
	else if(type == VK_IMAGE_TYPE_2D)
	{
		pImageFormatProperties->maxExtent.width = _limits.maxImageDimension2D;
		pImageFormatProperties->maxExtent.height = _limits.maxImageDimension2D;
		pImageFormatProperties->maxMipLevels = ulog2(_limits.maxImageDimension2D) + 1;
	}
	else
	{
		pImageFormatProperties->maxExtent.width = _limits.maxImageDimension3D;
		pImageFormatProperties->maxExtent.height = _limits.maxImageDimension3D;
		pImageFormatProperties->maxExtent.depth = _limits.maxImageDimension3D;
		pImageFormatProperties->maxMipLevels = ulog2(_limits.maxImageDimension3D) + 1;
	}

	int ycbcrConversionRequired = 0;

	for(uint32_t c = 0; c < numYcbcrConversionRequiredFormats; ++c)
	{
		if(format == ycbcrConversionRequiredFormats[c])
		{
			ycbcrConversionRequired = 1;
			break;
		}
	}

	if(ycbcrConversionRequired ||
	   tiling == VK_IMAGE_TILING_LINEAR ||
	   type != VK_IMAGE_TYPE_2D ||
	   flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ||
	   flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT
	   )
	{
		pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
	}

	//TODO real max size?
	//2^31
	pImageFormatProperties->maxResourceSize = 1<<31;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceImageFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
	VkImageFormatProperties2*                   pImageFormatProperties)
{
	assert(physicalDevice);
	assert(pImageFormatProperties);
	assert(pImageFormatInfo);

	//TODO handle pNext

	return vkGetPhysicalDeviceImageFormatProperties(physicalDevice,
													pImageFormatInfo->format,
													pImageFormatInfo->type,
													pImageFormatInfo->tiling,
													pImageFormatInfo->usage,
													pImageFormatInfo->flags,
													&pImageFormatProperties->imageFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateDeviceLayerProperties(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties)
{
	//deprecated, just return instance layers
	return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFeatures2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures2*                  pFeatures)
{
	assert(physicalDevice);
	assert(pFeatures);
	vkGetPhysicalDeviceFeatures(physicalDevice, &pFeatures->features);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceQueueFamilyProperties2(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pQueueFamilyPropertyCount,
	VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
	assert(physicalDevice);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
