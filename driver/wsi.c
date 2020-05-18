#include "common.h"
#include "modeset.h"

#include "kernel/vc4_packet.h"

#include "declarations.h"

#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkDisplayPlanePropertiesKHR*                pProperties)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR));

	assert(physicalDevice);
	assert(pPropertyCount);

	uint32_t numPlanes;
	modeset_plane planes[32];
	modeset_enum_planes(controlFd, &numPlanes, planes);

	if(!pProperties)
	{
		*pPropertyCount = numPlanes;
		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numPlanes, arraySize);

	for(uint32_t c = 0; c < elementsWritten; ++c)
	{
		pProperties[c].currentDisplay = planes[c].currentConnectorID;
		pProperties[c].currentStackIndex = c; //TODO dunno?
	}

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t                                    planeIndex,
	uint32_t*                                   pDisplayCount,
	VkDisplayKHR*                               pDisplays)
{
	PROFILESTART(RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR));

	assert(physicalDevice);
	assert(pDisplayCount);

	uint32_t numPlanes;
	modeset_plane planes[32];
	modeset_enum_planes(controlFd, &numPlanes, planes);

	if(!pDisplays)
	{
		*pDisplayCount = planes[planeIndex].numPossibleConnectors;

		PROFILEEND(RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pDisplayCount;
	int elementsWritten = min(planes[planeIndex].numPossibleConnectors, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pDisplays[c] = planes[planeIndex].possibleConnectors[c];
	}

	*pDisplayCount = elementsWritten;

	if(arraySize < planes[planeIndex].numPossibleConnectors)
	{
		PROFILEEND(RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkDisplayPropertiesKHR*                     pProperties)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR));

	assert(physicalDevice);
	assert(pPropertyCount);

	uint32_t numDisplays;
	modeset_display displays[16];
	modeset_enum_displays(controlFd, &numDisplays, displays);

	if(!pProperties)
	{
		*pPropertyCount = numDisplays;

		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numDisplays, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c].display = displays[c].connectorID;
		//fprintf(stderr, "display id %i\n", pProperties[c].display );
		char* name = (char*)malloc(32);
		memcpy(name, displays[c].name, 32);
		pProperties[c].displayName = (const char*)name;
		pProperties[c].physicalDimensions.width = displays[c].mmWidth;
		pProperties[c].physicalDimensions.height = displays[c].mmHeight;
		pProperties[c].physicalResolution.width = displays[c].resWidth;
		pProperties[c].physicalResolution.height = displays[c].resHeight;
	}

	*pPropertyCount = elementsWritten;

	if(arraySize < numDisplays)
	{
		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetDisplayModePropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	uint32_t*                                   pPropertyCount,
	VkDisplayModePropertiesKHR*                 pProperties)
{
	PROFILESTART(RPIFUNC(vkGetDisplayModePropertiesKHR));

	assert(physicalDevice);
	assert(display);
	assert(pPropertyCount);

	uint32_t numModes;
	modeset_display_mode modes[1024];
	modeset_enum_modes_for_display(controlFd, display, &numModes, &modes);

	if(!pProperties)
	{
		*pPropertyCount = numModes;

		PROFILEEND(RPIFUNC(vkGetDisplayModePropertiesKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numModes, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		_displayMode mode = { modes[c].connectorID, modes[c].modeID };
		memcpy(&pProperties[c].displayMode, &mode, sizeof(_displayMode));
		pProperties[c].parameters.visibleRegion.width = modes[c].resWidth;
		pProperties[c].parameters.visibleRegion.height = modes[c].resHeight;
		pProperties[c].parameters.refreshRate = modes[c].refreshRate;
	}

	*pPropertyCount = elementsWritten;

	if(arraySize < numModes)
	{
		PROFILEEND(RPIFUNC(vkGetDisplayModePropertiesKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetDisplayModePropertiesKHR));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDisplayModeKHR)(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	const VkDisplayModeCreateInfoKHR*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDisplayModeKHR*                           pMode)
{
	PROFILESTART(RPIFUNC(vkCreateDisplayModeKHR));

	assert(physicalDevice);
	assert(display);

	uint32_t numModes;
	modeset_display_mode modes[1024];
	modeset_enum_modes_for_display(controlFd, display, &numModes, &modes);

	for(uint32_t c = 0; c < numModes; ++c)
	{
		if(modes[c].refreshRate == pCreateInfo->parameters.refreshRate &&
		   modes[c].resWidth == pCreateInfo->parameters.visibleRegion.width &&
		   modes[c].resHeight == pCreateInfo->parameters.visibleRegion.height)
		{
			_displayMode mode = { modes[c].connectorID, modes[c].modeID };

			memcpy(pMode, &mode, sizeof(_displayMode));
			break;
		}
	}

	PROFILEEND(RPIFUNC(vkCreateDisplayModeKHR));
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDisplayPlaneSurfaceKHR)(
	VkInstance                                  instance,
	const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSurfaceKHR*                               pSurface)
{
	PROFILESTART(RPIFUNC(vkCreateDisplayPlaneSurfaceKHR));

	assert(instance);
	assert(pSurface);

	_displayMode mode;
	memcpy(&mode, &pCreateInfo->displayMode, sizeof(_displayMode));

	modeset_display_surface* surface = ALLOCATE(sizeof(modeset_display_surface), 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
	modeset_create_surface_for_mode(controlFd, mode.connectorID, mode.modeID, surface);

	*pSurface = surface;

	PROFILEEND(RPIFUNC(vkCreateDisplayPlaneSurfaceKHR));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySurfaceKHR
 * Destroying a VkSurfaceKHR merely severs the connection between Vulkan and the native surface,
 * and does not imply destroying the native surface, closing a window, or similar behavior
 * (but we'll do so anyways...)
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySurfaceKHR)(
		VkInstance                                  instance,
		VkSurfaceKHR                                surface,
		const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroySurfaceKHR));

	assert(instance);

	if(surface)
	{
		modeset_destroy_surface(controlFd, surface);
	}

	FREE(surface);

	PROFILEEND(RPIFUNC(vkDestroySurfaceKHR));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfaceCapabilitiesKHR
 * The capabilities of a swapchain targetting a surface are the intersection of the capabilities of the WSI platform,
 * the native window or display, and the physical device. The resulting capabilities can be obtained with the queries listed
 * below in this section. Capabilities that correspond to image creation parameters are not independent of each other:
 * combinations of parameters that are not supported as reported by vkGetPhysicalDeviceImageFormatProperties are not supported
 * by the surface on that physical device, even if the capabilities taken individually are supported as part of some other parameter combinations.
 *
 * capabilities the specified device supports for a swapchain created for the surface
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR));

	assert(physicalDevice);
	assert(surface);
	assert(pSurfaceCapabilities);

	modeset_display_surface* surf = surface;

	uint32_t width = surf->connector->modes[surf->modeID].hdisplay;
	uint32_t height = surf->connector->modes[surf->modeID].vdisplay;

	pSurfaceCapabilities->minImageCount = 1;
	pSurfaceCapabilities->maxImageCount = 2; //TODO max 2 for double buffering for now...
	pSurfaceCapabilities->currentExtent.width = width;
	pSurfaceCapabilities->currentExtent.height = height;
	pSurfaceCapabilities->minImageExtent.width = width; //TODO
	pSurfaceCapabilities->minImageExtent.height = height; //TODO
	pSurfaceCapabilities->maxImageExtent.width = width; //TODO
	pSurfaceCapabilities->maxImageExtent.height = height; //TODO
	pSurfaceCapabilities->maxImageArrayLayers = 1;
	pSurfaceCapabilities->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //TODO no rotation for now
	pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //TODO get this from dev
	pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //TODO no alpha compositing for now
	pSurfaceCapabilities->supportedUsageFlags =
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | //well we want to draw on the screen right
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |  //for clears
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  //for screenshots

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfaceFormatsKHR
 * If pSurfaceFormats is NULL, then the number of format pairs supported for the given surface is returned in pSurfaceFormatCount.
 * The number of format pairs supported will be greater than or equal to 1. Otherwise, pSurfaceFormatCount must point to a variable
 * set by the user to the number of elements in the pSurfaceFormats array, and on return the variable is overwritten with the number
 * of structures actually written to pSurfaceFormats. If the value of pSurfaceFormatCount is less than the number of format pairs supported,
 * at most pSurfaceFormatCount structures will be written. If pSurfaceFormatCount is smaller than the number of format pairs supported for the given surface,
 * VK_INCOMPLETE will be returned instead of VK_SUCCESS to indicate that not all the available values were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pSurfaceFormatCount,
		VkSurfaceFormatKHR*                         pSurfaceFormats)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR));

	assert(physicalDevice);
	assert(surface);
	assert(pSurfaceFormatCount);

	const int numFormats = 1;

	if(!pSurfaceFormats)
	{
		*pSurfaceFormatCount = numFormats;

		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pSurfaceFormatCount;
	int elementsWritten = min(numFormats, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pSurfaceFormats[c] = supportedSurfaceFormats[c];
	}

	*pSurfaceFormatCount = elementsWritten;

	if(elementsWritten < numFormats)
	{
		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfacePresentModesKHR
 * If pPresentModes is NULL, then the number of presentation modes supported for the given surface is returned in pPresentModeCount.
 * Otherwise, pPresentModeCount must point to a variable set by the user to the number of elements in the pPresentModes array,
 * and on return the variable is overwritten with the number of values actually written to pPresentModes.
 * If the value of pPresentModeCount is less than the number of presentation modes supported, at most pPresentModeCount values will be written.
 * If pPresentModeCount is smaller than the number of presentation modes supported for the given surface, VK_INCOMPLETE will be returned instead of
 * VK_SUCCESS to indicate that not all the available values were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pPresentModeCount,
		VkPresentModeKHR*                           pPresentModes)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR));

	assert(physicalDevice);
	assert(surface);
	assert(pPresentModeCount);

	if(!pPresentModes)
	{
		*pPresentModeCount = numSupportedPresentModes;

		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pPresentModeCount;
	int elementsWritten = min(numSupportedPresentModes, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		//TODO
		pPresentModes[c] = supportedPresentModes[c];
	}

	*pPresentModeCount = elementsWritten;

	if(elementsWritten < numSupportedPresentModes)
	{
		PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateSwapchainKHR
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateSwapchainKHR)(
		VkDevice                                    device,
		const VkSwapchainCreateInfoKHR*             pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSwapchainKHR*                             pSwapchain)
{
	PROFILESTART(RPIFUNC(vkCreateSwapchainKHR));

	assert(device);
	assert(pCreateInfo);
	assert(pSwapchain);

	*pSwapchain = ALLOCATE(sizeof(_swapchain), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!*pSwapchain)
	{
		PROFILEEND(RPIFUNC(vkCreateSwapchainKHR));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	_swapchain* s = *pSwapchain;

	//TODO flags, layers, queue sharing, pretransform, composite alpha..., clipped, oldswapchain
	//TODO present mode

	s->images = ALLOCATE(sizeof(_image) * pCreateInfo->minImageCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!s->images)
	{
		PROFILEEND(RPIFUNC(vkCreateSwapchainKHR));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	s->backbufferIdx = 0;
	s->numImages = pCreateInfo->minImageCount;
	s->surface = pCreateInfo->surface;

	for(int c = 0; c < pCreateInfo->minImageCount; ++c)
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = pCreateInfo->imageFormat;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = pCreateInfo->imageArrayLayers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = pCreateInfo->imageUsage;
		imageCreateInfo.sharingMode = pCreateInfo->imageSharingMode;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
		imageCreateInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
		imageCreateInfo.extent.width = pCreateInfo->imageExtent.width;
		imageCreateInfo.extent.height = pCreateInfo->imageExtent.height;
		imageCreateInfo.extent.depth = 1;

		VkImage img;
		RPIFUNC(vkCreateImage)(device, &imageCreateInfo, pAllocator, &img);

		s->images[c] = *(_image*)img;

		s->images[c].imageSpace = pCreateInfo->imageColorSpace;
		s->images[c].preTransformMode = pCreateInfo->preTransform;
		s->images[c].compositeAlpha = pCreateInfo->compositeAlpha;
		s->images[c].presentMode = pCreateInfo->presentMode;
		s->images[c].clipped = pCreateInfo->clipped;

		VkMemoryRequirements mr;
		RPIFUNC(vkGetImageMemoryRequirements)(device, &s->images[c], &mr);

		s->images[c].alignment = mr.alignment;

		VkMemoryAllocateInfo ai;
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = mr.size;
		for(int d = 0; d < numMemoryTypes; ++d)
		{
			if(memoryTypes[d].propertyFlags == mr.memoryTypeBits)
			{
				ai.memoryTypeIndex = d;
				break;
			}
		}

		VkDeviceMemory mem;
		RPIFUNC(vkAllocateMemory)(device, &ai, pAllocator, &mem);

		RPIFUNC(vkBindImageMemory)(device, &s->images[c], mem, 0);

		modeset_create_fb_for_surface(controlFd, &s->images[c], pCreateInfo->surface); assert(s->images[c].fb);
	}

	PROFILEEND(RPIFUNC(vkCreateSwapchainKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetSwapchainImagesKHR
 * If pSwapchainImages is NULL, then the number of presentable images for swapchain is returned in pSwapchainImageCount.
 * Otherwise, pSwapchainImageCount must point to a variable set by the user to the number of elements in the pSwapchainImages array,
 * and on return the variable is overwritten with the number of structures actually written to pSwapchainImages.
 * If the value of pSwapchainImageCount is less than the number of presentable images for swapchain, at most pSwapchainImageCount structures will be written.
 * If pSwapchainImageCount is smaller than the number of presentable images for swapchain, VK_INCOMPLETE will be returned instead of VK_SUCCESS to
 * indicate that not all the available values were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetSwapchainImagesKHR)(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint32_t*                                   pSwapchainImageCount,
		VkImage*                                    pSwapchainImages)
{
	PROFILESTART(RPIFUNC(vkGetSwapchainImagesKHR));

	assert(device);
	assert(swapchain);
	assert(pSwapchainImageCount);

	_swapchain* s = swapchain;

	if(!pSwapchainImages)
	{
		*pSwapchainImageCount = s->numImages;

		PROFILEEND(RPIFUNC(vkGetSwapchainImagesKHR));
		return VK_SUCCESS;
	}

	int arraySize = *pSwapchainImageCount;
	int elementsWritten = min(s->numImages, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pSwapchainImages[c] = &s->images[c];
	}

	*pSwapchainImageCount = elementsWritten;

	if(elementsWritten < s->numImages)
	{
		PROFILEEND(RPIFUNC(vkGetSwapchainImagesKHR));
		return VK_INCOMPLETE;
	}

	PROFILEEND(RPIFUNC(vkGetSwapchainImagesKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAcquireNextImageKHR
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAcquireNextImageKHR)(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint64_t                                    timeout,
		VkSemaphore                                 semaphore,
		VkFence                                     fence,
		uint32_t*                                   pImageIndex)
{
	PROFILESTART(RPIFUNC(vkAcquireNextImageKHR));

	assert(device);
	assert(swapchain);

	assert(semaphore != VK_NULL_HANDLE || fence != VK_NULL_HANDLE);

	sem_t* s = semaphore;

	//TODO we need to keep track of currently acquired images?

	//TODO wait timeout?

	*pImageIndex = ((_swapchain*)swapchain)->backbufferIdx; //return back buffer index

	//signal semaphore
	int semVal; sem_getvalue(s, &semVal); assert(semVal <= 0); //make sure semaphore is unsignalled
	sem_post(s);

	_fence* f = fence;
	if(f)
	{
		f->signaled = 1;
	}

	PROFILEEND(RPIFUNC(vkAcquireNextImageKHR));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkQueuePresentKHR
 * Any writes to memory backing the images referenced by the pImageIndices and pSwapchains members of pPresentInfo,
 * that are available before vkQueuePresentKHR is executed, are automatically made visible to the read access performed by the presentation engine.
 * This automatic visibility operation for an image happens-after the semaphore signal operation, and happens-before the presentation engine accesses the image.
 * Queueing an image for presentation defines a set of queue operations, including waiting on the semaphores and submitting a presentation request to the presentation engine.
 * However, the scope of this set of queue operations does not include the actual processing of the image by the presentation engine.
 * If vkQueuePresentKHR fails to enqueue the corresponding set of queue operations, it may return VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY.
 * If it does, the implementation must ensure that the state and contents of any resources or synchronization primitives referenced is unaffected by the call or its failure.
 * If vkQueuePresentKHR fails in such a way that the implementation is unable to make that guarantee, the implementation must return VK_ERROR_DEVICE_LOST.
 * However, if the presentation request is rejected by the presentation engine with an error VK_ERROR_OUT_OF_DATE_KHR or VK_ERROR_SURFACE_LOST_KHR,
 * the set of queue operations are still considered to be enqueued and thus any semaphore to be waited on gets unsignaled when the corresponding queue operation is complete.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueuePresentKHR)(
		VkQueue                                     queue,
		const VkPresentInfoKHR*                     pPresentInfo)
{
	PROFILESTART(RPIFUNC(vkQueuePresentKHR));

	assert(queue);
	assert(pPresentInfo);

	//wait for semaphore in present info set by submit ioctl to make sure cls are flushed
	for(int c = 0; c < pPresentInfo->waitSemaphoreCount; ++c)
	{
		sem_wait((sem_t*)pPresentInfo->pWaitSemaphores[c]);
	}

	for(int c = 0; c < pPresentInfo->swapchainCount; ++c)
	{
		_swapchain* s = pPresentInfo->pSwapchains[c];
		modeset_present(controlFd, &s->images[s->backbufferIdx], s->surface);
		s->backbufferIdx = (s->backbufferIdx + 1) % s->numImages;
	}

	PROFILEEND(RPIFUNC(vkQueuePresentKHR));

	endFrame();

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySwapchainKHR
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySwapchainKHR)(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroySwapchainKHR));

	assert(device);

	//TODO flush all ops

	_swapchain* s = swapchain;

	if(s)
	{
		for(int c = 0; c < s->numImages; ++c)
		{
			RPIFUNC(vkFreeMemory)(device, s->images[c].boundMem, 0);
			modeset_destroy_fb(controlFd, &s->images[c]);
		}

		FREE(s->images);
	}

	FREE(s);

	PROFILEEND(RPIFUNC(vkDestroySwapchainKHR));

	profilePrintResults();
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfaceSupportKHR
 * does this queue family support presentation to this surface?
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceSupportKHR)(
		VkPhysicalDevice                            physicalDevice,
		uint32_t                                    queueFamilyIndex,
		VkSurfaceKHR                                surface,
		VkBool32*                                   pSupported)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceSurfaceSupportKHR));

	assert(pSupported);
	assert(surface);
	assert(physicalDevice);

	assert(queueFamilyIndex < numQueueFamilies);

	//TODO if we plan to support headless rendering, there should be 2 families
	//one using /dev/dri/card0 which has modesetting
	//other using /dev/dri/renderD128 which does not support modesetting, this would say false here
	*pSupported = VK_TRUE;

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceSurfaceSupportKHR));
	return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif
