#include "common.h"

#include <vulkan/vk_icd.h>

#include "declarations.h"

#if EXPOSE_DRIVER == 0
	#define RETFUNC(f) if(!strcmp(pName, #f)) return &rpi_##f
#else
	#define RETFUNC(f) if(!strcmp(pName, #f)) return &f
#endif

static uint32_t loaderVersion = -1;

VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion)
{
	assert(pSupportedVersion);
	loaderVersion = *pSupportedVersion;

	*pSupportedVersion = 4; //we support v4

	return VK_SUCCESS;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName)
{
	if(loaderVersion == -1)
	{
		//dealing with legacy ICD loader, as vk_icdNegotiateLoaderICDInterfaceVersion has not been called
		loaderVersion = 1;
	}

	void* ptr = RPIFUNC(vkGetInstanceProcAddr)(instance, pName);

	return ptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(VkInstance instance,
												   const char* pName)
{
	void* ptr = 0;

//	if(!strcmp(pName, "vkCreateShaderModuleFromRpiAssemblyEXT"))
//		ptr = &RPIFUNC(vkCreateShaderModuleFromRpiAssemblyEXT);

	return ptr;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceExtensionProperties
 * When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation or by implicitly enabled layers are returned. When pLayerName is the name of a layer,
 * the instance extensions provided by that layer are returned.
 * If pProperties is NULL, then the number of extensions properties available is returned in pPropertyCount. Otherwise, pPropertyCount must point to a variable set by the user
 * to the number of elements in the pProperties array, and on return the variable is overwritten with the number of structures actually written to pProperties.
 * If pPropertyCount is less than the number of extension properties available, at most pPropertyCount structures will be written. If pPropertyCount is smaller than the number of extensions available,
 * VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the available properties were returned.
 * Because the list of available layers may change externally between calls to vkEnumerateInstanceExtensionProperties,
 * two calls may retrieve different results if a pLayerName is available in one call but not in another. The extensions supported by a layer may also change between two calls,
 * e.g. if the layer implementation is replaced by a different version between those calls.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceExtensionProperties)(
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	PROFILESTART(RPIFUNC(vkEnumerateInstanceExtensionProperties));

	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numInstanceExtensions;
		PROFILEEND(RPIFUNC(vkEnumerateInstanceExtensionProperties));
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numInstanceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = instanceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	if(arraySize < numInstanceExtensions)
	{
		PROFILEEND(RPIFUNC(vkEnumerateInstanceExtensionProperties));
		return VK_INCOMPLETE;
	}
	else
	{
		PROFILEEND(RPIFUNC(vkEnumerateInstanceExtensionProperties));
		return VK_SUCCESS;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateInstance
 * There is no global state in Vulkan and all per-application state is stored in a VkInstance object. Creating a VkInstance object initializes the Vulkan library
 * vkCreateInstance verifies that the requested layers exist. If not, vkCreateInstance will return VK_ERROR_LAYER_NOT_PRESENT. Next vkCreateInstance verifies that
 * the requested extensions are supported (e.g. in the implementation or in any enabled instance layer) and if any requested extension is not supported,
 * vkCreateInstance must return VK_ERROR_EXTENSION_NOT_PRESENT. After verifying and enabling the instance layers and extensions the VkInstance object is
 * created and returned to the application.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateInstance)(
		const VkInstanceCreateInfo*                 pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkInstance*                                 pInstance)
{
	PROFILESTART(RPIFUNC(vkCreateInstance));

	assert(pInstance);
	assert(pCreateInfo);

	*pInstance = ALLOCATE(sizeof(_instance), 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

	if(!*pInstance)
	{
		PROFILEEND(RPIFUNC(vkCreateInstance));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	set_loader_magic_value(&(*pInstance)->loaderData);

	(*pInstance)->numEnabledExtensions = 0;

	if(pCreateInfo->enabledExtensionCount)
	{
		assert(pCreateInfo->ppEnabledExtensionNames);
	}

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findInstanceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pInstance)->enabledExtensions[(*pInstance)->numEnabledExtensions] = findres;
			(*pInstance)->numEnabledExtensions++;
		}
		else
		{
			FREE(*pInstance);
			*pInstance = 0;
			PROFILEEND(RPIFUNC(vkCreateInstance));
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//TODO ignored for now
	//pCreateInfo->pApplicationInfo

	{ //Simple check to make sure we only support RPi 0, 1, 2, 3
		FILE* f = fopen("/proc/cpuinfo", "r");

		if(!f)
		{
			PROFILEEND(RPIFUNC(vkCreateInstance));
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		char* str = malloc(4096);
		int n = fread(str, 1, 4096, f);
		fclose(f);
		str[n] = '\0';

		char* hw = strstr(str, "Hardware");
		hw = strstr(hw, "BCM");

		hw[7] = '\0';

		if(strcmp(hw, "BCM2835") &&
		   strcmp(hw, "BCM2836") &&
		   strcmp(hw, "BCM2837"))
		{
			PROFILEEND(RPIFUNC(vkCreateInstance));
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		free(str);
	}


	//we assume we are on the RPi and the GPU exists...
	int gpuExists = access( "/dev/dri/card0", F_OK ) != -1; assert(gpuExists);

	(*pInstance)->dev.path = "/dev/dri/card0";
	(*pInstance)->dev.instance = *pInstance;

	set_loader_magic_value(&(*pInstance)->dev.loaderData);

	int ret = openIoctl();
	if(ret == -1)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	ret = vc4_get_chip_info(controlFd,
					  &(*pInstance)->technologyVersion,
					  &(*pInstance)->IDstrUINT,
					  &(*pInstance)->vpmMemorySize,
					  &(*pInstance)->hdrSupported,
					  &(*pInstance)->numSemaphores,
					  &(*pInstance)->numTMUperSlice,
					  &(*pInstance)->numQPUperSlice,
					  &(*pInstance)->numSlices,
					  &(*pInstance)->v3dRevision,
					  &(*pInstance)-> tileBufferDoubleBufferModeSupported,
					  &(*pInstance)->tileBufferSize,
					  &(*pInstance)->vriMemorySize);
	if(!ret)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	(*pInstance)->hasTiling = vc4_test_tiling(controlFd);

	(*pInstance)->hasControlFlow = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
	(*pInstance)->hasEtc1 = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_ETC1);
	(*pInstance)->hasThreadedFs = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
	(*pInstance)->hasMadvise = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_MADVISE);
	(*pInstance)->hasPerfmon = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_PERFMON);
	(*pInstance)->hasFixedRCLorder = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_FIXED_RCL_ORDER);

	char IDstring[] = { 0, 0, 0, 0 };
	memcpy(IDstring, &(*pInstance)->IDstrUINT, 3);

//	printf("------------------------------------------\n");
//	printf("------------------------------------------\n");
//	printf("V3D chip info: \n");
//	printf("IDstring %s\n", IDstring);
//	printf("technologyVersion: %u\n", (*pInstance)->technologyVersion);
//	printf("v3dRevision %u\n", (*pInstance)->v3dRevision);
//	printf("vpmMemorySize %u\n", (*pInstance)->vpmMemorySize);
//	printf("numSemaphores %u\n", (*pInstance)->numSemaphores);
//	printf("numTMUperSlice %u\n", (*pInstance)->numTMUperSlice);
//	printf("numQPUperSlice %u\n", (*pInstance)->numQPUperSlice);
//	printf("numSlices %u\n", (*pInstance)->numSlices);
//	printf("tileBufferSize %s\n", (*pInstance)->tileBufferSize > 0 ?
//			   (*pInstance)->tileBufferSize > 1 ? "full" : "half" : "quarter");
//	printf("vriMemorySize %s\n", (*pInstance)->vriMemorySize ? "full" : "half");
//	printf("hdrSupported %u\n", (*pInstance)->hdrSupported);
//	printf("tileBufferDoubleBufferModeSupported %u\n", (*pInstance)-> tileBufferDoubleBufferModeSupported);
//	printf("hasTiling %u\n", (*pInstance)->hasTiling);
//	printf("hasControlFlow %u\n", (*pInstance)->hasControlFlow);
//	printf("hasEtc1 %u\n", (*pInstance)->hasEtc1);
//	printf("hasThreadedFs %u\n", (*pInstance)->hasThreadedFs);
//	printf("hasMadvise %u\n", (*pInstance)->hasMadvise);
//	printf("hasPerfmon %u\n", (*pInstance)->hasPerfmon);
//	printf("hasFixedRCLorder %u\n", (*pInstance)->hasFixedRCLorder);
//	printf("------------------------------------------\n");
//	printf("------------------------------------------\n");

	assert((*pInstance)->hasTiling);
	assert((*pInstance)->hasControlFlow);
	assert((*pInstance)->hasEtc1);
	assert((*pInstance)->hasThreadedFs);
	assert((*pInstance)->hasPerfmon);

	PROFILEEND(RPIFUNC(vkCreateInstance));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyInstance
 *
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyInstance)(
		VkInstance                                  instance,
		const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyInstance));
	if(instance)
	{
		closeIoctl();

		FREE(instance);
	}
	PROFILEEND(RPIFUNC(vkDestroyInstance));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceVersion
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceVersion)(
	uint32_t*                                   pApiVersion)
{
	PROFILESTART(RPIFUNC(vkEnumerateInstanceVersion));
	assert(pApiVersion);
	*pApiVersion = VK_DRIVER_VERSION; //
	PROFILEEND(RPIFUNC(vkEnumerateInstanceVersion));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetInstanceProcAddr
 */
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL RPIFUNC(vkGetInstanceProcAddr)(
	VkInstance                                  instance,
	const char*                                 pName)
{
	PROFILESTART(RPIFUNC(vkGetInstanceProcAddr));

	if(!instance && !(
		   !strcmp(pName, "vkEnumerateInstanceVersion") ||
		   !strcmp(pName, "vkEnumerateInstanceExtensionProperties") ||
		   !strcmp(pName, "vkEnumerateInstanceLayerProperties") ||
		   !strcmp(pName, "vkCreateInstance")
		   ))
	{
		PROFILEEND(RPIFUNC(vkGetInstanceProcAddr));
		return 0;
	}

	PROFILEEND(RPIFUNC(vkGetInstanceProcAddr));

	RETFUNC(vkCreateInstance);
	RETFUNC(vkEnumerateInstanceVersion);
	RETFUNC(vkDestroyInstance);
	RETFUNC(vkEnumeratePhysicalDevices);
	RETFUNC(vkGetPhysicalDeviceFeatures);
	RETFUNC(vkGetPhysicalDeviceFormatProperties);
	RETFUNC(vkGetPhysicalDeviceImageFormatProperties);
	RETFUNC(vkGetPhysicalDeviceProperties);
	RETFUNC(vkGetPhysicalDeviceQueueFamilyProperties);
	RETFUNC(vkGetPhysicalDeviceMemoryProperties);
	RETFUNC(vkGetInstanceProcAddr);
	RETFUNC(vkGetDeviceProcAddr);
	RETFUNC(vkCreateDevice);
	RETFUNC(vkDestroyDevice);
	RETFUNC(vkEnumerateInstanceExtensionProperties);
	RETFUNC(vkEnumerateDeviceExtensionProperties);
	RETFUNC(vkEnumerateInstanceLayerProperties);
	RETFUNC(vkEnumerateDeviceLayerProperties);
	RETFUNC(vkGetDeviceQueue);
	RETFUNC(vkQueueSubmit);
	RETFUNC(vkQueueWaitIdle);
	RETFUNC(vkDeviceWaitIdle);
	RETFUNC(vkAllocateMemory);
	RETFUNC(vkFreeMemory);
	RETFUNC(vkMapMemory);
	RETFUNC(vkUnmapMemory);
	RETFUNC(vkFlushMappedMemoryRanges);
	RETFUNC(vkInvalidateMappedMemoryRanges);
	RETFUNC(vkGetDeviceMemoryCommitment);
	RETFUNC(vkBindBufferMemory);
	RETFUNC(vkBindImageMemory);
	RETFUNC(vkGetBufferMemoryRequirements);
	RETFUNC(vkGetImageMemoryRequirements);
	RETFUNC(vkGetImageSparseMemoryRequirements);
	RETFUNC(vkGetPhysicalDeviceSparseImageFormatProperties);
	RETFUNC(vkQueueBindSparse);
	RETFUNC(vkCreateFence);
	RETFUNC(vkDestroyFence);
	RETFUNC(vkResetFences);
	RETFUNC(vkGetFenceStatus);
	RETFUNC(vkWaitForFences);
	RETFUNC(vkCreateSemaphore);
	RETFUNC(vkDestroySemaphore);
	RETFUNC(vkCreateEvent);
	RETFUNC(vkDestroyEvent);
	RETFUNC(vkGetEventStatus);
	RETFUNC(vkSetEvent);
	RETFUNC(vkResetEvent);
	RETFUNC(vkCreateQueryPool);
	RETFUNC(vkDestroyQueryPool);
	RETFUNC(vkGetQueryPoolResults);
	RETFUNC(vkCreateBuffer);
	RETFUNC(vkDestroyBuffer);
	RETFUNC(vkCreateBufferView);
	RETFUNC(vkDestroyBufferView);
	RETFUNC(vkCreateImage);
	RETFUNC(vkDestroyImage);
	RETFUNC(vkGetImageSubresourceLayout);
	RETFUNC(vkCreateImageView);
	RETFUNC(vkDestroyImageView);
	RETFUNC(vkCreateShaderModule);
	RETFUNC(vkDestroyShaderModule);
	RETFUNC(vkCreatePipelineCache);
	RETFUNC(vkDestroyPipelineCache);
	RETFUNC(vkGetPipelineCacheData);
	RETFUNC(vkMergePipelineCaches);
	RETFUNC(vkCreateGraphicsPipelines);
	RETFUNC(vkCreateComputePipelines);
	RETFUNC(vkDestroyPipeline);
	RETFUNC(vkCreatePipelineLayout);
	RETFUNC(vkDestroyPipelineLayout);
	RETFUNC(vkCreateSampler);
	RETFUNC(vkDestroySampler);
	RETFUNC(vkCreateDescriptorSetLayout);
	RETFUNC(vkDestroyDescriptorSetLayout);
	RETFUNC(vkCreateDescriptorPool);
	RETFUNC(vkDestroyDescriptorPool);
	RETFUNC(vkResetDescriptorPool);
	RETFUNC(vkAllocateDescriptorSets);
	RETFUNC(vkFreeDescriptorSets);
	RETFUNC(vkUpdateDescriptorSets);
	RETFUNC(vkCreateFramebuffer);
	RETFUNC(vkDestroyFramebuffer);
	RETFUNC(vkCreateRenderPass);
	RETFUNC(vkDestroyRenderPass);
	RETFUNC(vkGetRenderAreaGranularity);
	RETFUNC(vkCreateCommandPool);
	RETFUNC(vkDestroyCommandPool);
	RETFUNC(vkResetCommandPool);
	RETFUNC(vkAllocateCommandBuffers);
	RETFUNC(vkFreeCommandBuffers);
	RETFUNC(vkBeginCommandBuffer);
	RETFUNC(vkEndCommandBuffer);
	RETFUNC(vkResetCommandBuffer);
	RETFUNC(vkCmdBindPipeline);
	RETFUNC(vkCmdSetViewport);
	RETFUNC(vkCmdSetScissor);
	RETFUNC(vkCmdSetLineWidth);
	RETFUNC(vkCmdSetDepthBias);
	RETFUNC(vkCmdSetBlendConstants);
	RETFUNC(vkCmdSetDepthBounds);
	RETFUNC(vkCmdSetStencilCompareMask);
	RETFUNC(vkCmdSetStencilWriteMask);
	RETFUNC(vkCmdSetStencilReference);
	RETFUNC(vkCmdBindDescriptorSets);
	RETFUNC(vkCmdBindIndexBuffer);
	RETFUNC(vkCmdBindVertexBuffers);
	RETFUNC(vkCmdDraw);
	RETFUNC(vkCmdDrawIndexed);
	RETFUNC(vkCmdDrawIndirect);
	RETFUNC(vkCmdDrawIndexedIndirect);
	RETFUNC(vkCmdDispatch);
	RETFUNC(vkCmdDispatchIndirect);
	RETFUNC(vkCmdCopyBuffer);
	RETFUNC(vkCmdCopyImage);
	RETFUNC(vkCmdBlitImage);
	RETFUNC(vkCmdCopyBufferToImage);
	RETFUNC(vkCmdCopyImageToBuffer);
	RETFUNC(vkCmdUpdateBuffer);
	RETFUNC(vkCmdFillBuffer);
	RETFUNC(vkCmdClearColorImage);
	RETFUNC(vkCmdClearDepthStencilImage);
	RETFUNC(vkCmdClearAttachments);
	RETFUNC(vkCmdResolveImage);
	RETFUNC(vkCmdSetEvent);
	RETFUNC(vkCmdResetEvent);
	RETFUNC(vkCmdWaitEvents);
	RETFUNC(vkCmdPipelineBarrier);
	RETFUNC(vkCmdBeginQuery);
	RETFUNC(vkCmdEndQuery);
	RETFUNC(vkCmdResetQueryPool);
	RETFUNC(vkCmdWriteTimestamp);
	RETFUNC(vkCmdCopyQueryPoolResults);
	RETFUNC(vkCmdPushConstants);
	RETFUNC(vkCmdBeginRenderPass);
	RETFUNC(vkCmdNextSubpass);
	RETFUNC(vkCmdEndRenderPass);
	RETFUNC(vkCmdExecuteCommands);
	RETFUNC(vkEnumeratePhysicalDeviceGroups);
	RETFUNC(vkGetPhysicalDeviceFeatures2);
	RETFUNC(vkGetPhysicalDeviceProperties2);
	RETFUNC(vkGetPhysicalDeviceFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceImageFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceQueueFamilyProperties2);
	RETFUNC(vkGetPhysicalDeviceMemoryProperties2);
	RETFUNC(vkGetPhysicalDeviceSparseImageFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceExternalBufferProperties);
	RETFUNC(vkGetPhysicalDeviceExternalFenceProperties);
	RETFUNC(vkGetPhysicalDeviceExternalSemaphoreProperties);
	RETFUNC(vkBindImageMemory2);
	RETFUNC(vkGetDeviceGroupPeerMemoryFeatures);
	RETFUNC(vkCmdSetDeviceMask);
	RETFUNC(vkCmdDispatchBase);
	RETFUNC(vkGetImageMemoryRequirements2);
	RETFUNC(vkGetBufferMemoryRequirements2);
	RETFUNC(vkGetImageSparseMemoryRequirements2);
	RETFUNC(vkTrimCommandPool);
	RETFUNC(vkGetDeviceQueue2);
	RETFUNC(vkCreateSamplerYcbcrConversion);
	RETFUNC(vkDestroySamplerYcbcrConversion);
	RETFUNC(vkCreateDescriptorUpdateTemplate);
	RETFUNC(vkDestroyDescriptorUpdateTemplate);
	RETFUNC(vkUpdateDescriptorSetWithTemplate);
	RETFUNC(vkGetDescriptorSetLayoutSupport);
	RETFUNC(vkBindBufferMemory2);

	RETFUNC(vkDestroySurfaceKHR);
	RETFUNC(vkCreateSwapchainKHR);
	RETFUNC(vkGetPhysicalDeviceSurfaceSupportKHR);
	RETFUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	RETFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
	RETFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);

	RETFUNC(vkGetSwapchainImagesKHR);
	RETFUNC(vkAcquireNextImageKHR);
	RETFUNC(vkQueuePresentKHR);

	RETFUNC(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
	RETFUNC(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR);
	RETFUNC(vkAcquireProfilingLockKHR);
	RETFUNC(vkReleaseProfilingLockKHR);

	RETFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR);
	RETFUNC(vkGetDisplayModePropertiesKHR);
	RETFUNC(vkCreateDisplayModeKHR);
	RETFUNC(vkCreateDisplayPlaneSurfaceKHR);
	RETFUNC(vkGetDisplayPlaneSupportedDisplaysKHR);
	RETFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);

	RETFUNC(vkDestroySwapchainKHR);

	return 0;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceLayerProperties)(
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties)
{
	PROFILESTART(RPIFUNC(vkEnumerateInstanceLayerProperties));


	PROFILEEND(RPIFUNC(vkEnumerateInstanceLayerProperties));
	return VK_SUCCESS;
}
