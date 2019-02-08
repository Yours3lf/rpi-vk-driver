#include "common.h"
#include "modeset.h"

#include "kernel/vc4_packet.h"

/*
 * Implementation of our RPI specific "extension"
 */
VkResult vkCreateRpiSurfaceKHR(
		VkInstance                                  instance,
		const VkRpiSurfaceCreateInfoKHR*            pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSurfaceKHR*                               pSurface)
{
	assert(instance);
	//assert(pCreateInfo); //ignored for now
	assert(pSurface);

	//TODO use allocator!

	*pSurface = (VkSurfaceKHR)modeset_create(controlFd);

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySurfaceKHR
 * Destroying a VkSurfaceKHR merely severs the connection between Vulkan and the native surface,
 * and does not imply destroying the native surface, closing a window, or similar behavior
 * (but we'll do so anyways...)
 */
VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(
		VkInstance                                  instance,
		VkSurfaceKHR                                surface,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(instance);
	assert(surface);

	//TODO use allocator

	modeset_destroy(controlFd, (modeset_dev*)surface);
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
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
	assert(physicalDevice);
	assert(surface);
	assert(pSurfaceCapabilities);

	pSurfaceCapabilities->minImageCount = 1; //min 1
	pSurfaceCapabilities->maxImageCount = 2; //TODO max 2 for double buffering for now...
	pSurfaceCapabilities->currentExtent.width = ((modeset_dev*)surface)->width;
	pSurfaceCapabilities->currentExtent.height = ((modeset_dev*)surface)->height;
	pSurfaceCapabilities->minImageExtent.width = ((modeset_dev*)surface)->width; //TODO
	pSurfaceCapabilities->minImageExtent.height = ((modeset_dev*)surface)->height; //TODO
	pSurfaceCapabilities->maxImageExtent.width = ((modeset_dev*)surface)->width; //TODO
	pSurfaceCapabilities->maxImageExtent.height = ((modeset_dev*)surface)->height; //TODO
	pSurfaceCapabilities->maxImageArrayLayers = 1; //TODO maybe more layers for cursor etc.
	pSurfaceCapabilities->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //TODO no rotation for now
	pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //TODO get this from dev
	pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //TODO no alpha compositing for now
	pSurfaceCapabilities->supportedUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //well we want to draw on the screen right

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
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pSurfaceFormatCount,
		VkSurfaceFormatKHR*                         pSurfaceFormats)
{
	assert(physicalDevice);
	assert(surface);
	assert(pSurfaceFormatCount);

	const int numFormats = 1;

	if(!pSurfaceFormats)
	{
		*pSurfaceFormatCount = numFormats;
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
		return VK_INCOMPLETE;
	}

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
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pPresentModeCount,
		VkPresentModeKHR*                           pPresentModes)
{
	assert(physicalDevice);
	assert(surface);
	assert(pPresentModeCount);

	const int numModes = 1;

	if(!pPresentModes)
	{
		*pPresentModeCount = numModes;
		return VK_SUCCESS;
	}

	int arraySize = *pPresentModeCount;
	int elementsWritten = min(numModes, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		//TODO
		pPresentModes[c] = VK_PRESENT_MODE_FIFO_KHR;
	}

	*pPresentModeCount = elementsWritten;

	if(elementsWritten < numModes)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateSwapchainKHR
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(
		VkDevice                                    device,
		const VkSwapchainCreateInfoKHR*             pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSwapchainKHR*                             pSwapchain)
{
	assert(device);
	assert(pCreateInfo);
	assert(pSwapchain);

	*pSwapchain = ALLOCATE(sizeof(_swapchain), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!*pSwapchain)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	_swapchain* s = *pSwapchain;

	//TODO flags, layers, queue sharing, pretransform, composite alpha, present mode..., clipped, oldswapchain
	//TODO external sync on surface, oldswapchain

	s->images = ALLOCATE(sizeof(_image) * pCreateInfo->minImageCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!s->images)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	s->backbufferIdx = 0;
	s->numImages = pCreateInfo->minImageCount;
	s->surface = pCreateInfo->surface;

	for(int c = 0; c < pCreateInfo->minImageCount; ++c)
	{
		s->images[c].width = pCreateInfo->imageExtent.width;
		s->images[c].height = pCreateInfo->imageExtent.height;
		s->images[c].depth = 1;
		s->images[c].layers = pCreateInfo->imageArrayLayers;
		s->images[c].miplevels = 1;
		s->images[c].samples = 1; //TODO
		s->images[c].usageBits = pCreateInfo->imageUsage;
		s->images[c].format = pCreateInfo->imageFormat;
		s->images[c].imageSpace = pCreateInfo->imageColorSpace;
		s->images[c].concurrentAccess = pCreateInfo->imageSharingMode;
		s->images[c].numQueueFamiliesWithAccess = pCreateInfo->queueFamilyIndexCount;
		if(s->images[c].concurrentAccess)
		{
			s->images[c].queueFamiliesWithAccess = ALLOCATE(sizeof(uint32_t)*s->images[c].numQueueFamiliesWithAccess, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!s->images[c].queueFamiliesWithAccess)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}
			memcpy(s->images[c].queueFamiliesWithAccess, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t)*s->images[c].numQueueFamiliesWithAccess);
		}
		s->images[c].preTransformMode = pCreateInfo->preTransform;
		s->images[c].compositeAlpha = pCreateInfo->compositeAlpha;
		s->images[c].presentMode = pCreateInfo->presentMode;
		s->images[c].clipped = pCreateInfo->clipped;


		VkMemoryRequirements mr;
		vkGetImageMemoryRequirements(device, &s->images[c], &mr);

		//TODO is this the right place to do this?
		s->images[c].tiling = VC4_TILING_FORMAT_T;
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
		vkAllocateMemory(device, &ai, 0, &mem);

		vkBindImageMemory(device, &s->images[c], mem, 0);

		//set tiling to T if size > 4KB
		if(s->images[c].tiling == VC4_TILING_FORMAT_T)
		{
			int ret = vc4_bo_set_tiling(controlFd, s->images[c].boundMem->bo, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED); assert(ret);
		}
		else
		{
			int ret = vc4_bo_set_tiling(controlFd, s->images[c].boundMem->bo, DRM_FORMAT_MOD_LINEAR); assert(ret);
		}

		int res = modeset_create_fb(controlFd, &s->images[c]); assert(res == 0);
	}

	//defer to first swapbuffer (or at least later, getting swapchain != presenting immediately)
	//int res = modeset_fb_for_dev(controlFd, s->surface, &s->images[s->backbufferIdx]); assert(res == 0);

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
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint32_t*                                   pSwapchainImageCount,
		VkImage*                                    pSwapchainImages)
{
	assert(device);
	assert(swapchain);
	assert(pSwapchainImageCount);

	_swapchain* s = swapchain;

	if(!pSwapchainImages)
	{
		*pSwapchainImageCount = s->numImages;
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
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAcquireNextImageKHR
 */
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint64_t                                    timeout,
		VkSemaphore                                 semaphore,
		VkFence                                     fence,
		uint32_t*                                   pImageIndex)
{
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

	//TODO signal fence

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
VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(
		VkQueue                                     queue,
		const VkPresentInfoKHR*                     pPresentInfo)
{
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
		modeset_present_buffer(controlFd, (modeset_dev*)s->surface, &s->images[s->backbufferIdx]);
		s->backbufferIdx = (s->backbufferIdx + 1) % s->numImages;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySwapchainKHR
 */
VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(swapchain);

	//TODO flush all ops

	_swapchain* s = swapchain;

	for(int c = 0; c < s->numImages; ++c)
	{
		vkFreeMemory(device, s->images[c].boundMem, 0);
		modeset_destroy_fb(controlFd, &s->images[c]);
	}

	FREE(s->images);
	FREE(s);
}

