#include <stdio.h>
#include "CustomAssert.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <drm/vc4_drm.h>

#include <vulkan/vulkan.h>
#include "vkExt.h"

#include "modeset.h"

#include "PoolAllocator.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

#define DRM_IOCTL_FILE_NAME "/dev/"DRM_NAME

static int fd = -1;

int openIoctl()
{
	fd = open(DRM_IOCTL_FILE_NAME, O_RDWR);
	if (fd < 0) {
		printf("Can't open device file: %s\n", DRM_IOCTL_FILE_NAME);
		return -1;
	}

	return 0;
}

void closeIoctl(int fd)
{
	close(fd);
}

typedef struct VkInstance_T
{
	//supposedly this should contain all the enabled layers?
	int dummy;
} _instance;

typedef struct VkPhysicalDevice_T
{
	//hardware id?
	int dummy;
} _physicalDevice;

typedef struct VkDevice_T
{
	int dummy;
} _device;

typedef struct VkQueue_T
{
	int familyIndex;
} _queue;

typedef struct VkCommandBuffer_T
{
	int dummy;
} _commandBuffer;

VkQueueFamilyProperties _queueFamilyProperties[] =
{
	{
		//TODO maybe sparse textures later?
		.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		.queueCount = 1,
		.timestampValidBits = 32, //TODO dunno, 32 for now
		.minImageTransferGranularity = {1, 1, 1}
	}
};
const int numQueueFamilies = sizeof(_queueFamilyProperties)/sizeof(VkQueueFamilyProperties);

_queue _queuesByFamily[][1] =
{
	{
		{
			.familyIndex = 0
		}
	}
};

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateInstance
 * There is no global state in Vulkan and all per-application state is stored in a VkInstance object. Creating a VkInstance object initializes the Vulkan library
 * vkCreateInstance verifies that the requested layers exist. If not, vkCreateInstance will return VK_ERROR_LAYER_NOT_PRESENT. Next vkCreateInstance verifies that
 * the requested extensions are supported (e.g. in the implementation or in any enabled instance layer) and if any requested extension is not supported,
 * vkCreateInstance must return VK_ERROR_EXTENSION_NOT_PRESENT. After verifying and enabling the instance layers and extensions the VkInstance object is
 * created and returned to the application.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
	const VkInstanceCreateInfo*                 pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkInstance*                                 pInstance)
{
	*pInstance = malloc(sizeof(_instance));
	assert(pInstance);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO: possibly we need to load layers here
	//and store them in pInstance

	//TODO: need to check here that the requested
	//extensions are supported
	//eg.
	//VK_KHR_surface

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#devsandqueues-physical-device-enumeration
 * If pPhysicalDevices is NULL, then the number of physical devices available is returned in pPhysicalDeviceCount. Otherwise, pPhysicalDeviceCount must point to a
 * variable set by the user to the number of elements in the pPhysicalDevices array, and on return the variable is overwritten with the number of handles actually
 * written to pPhysicalDevices. If pPhysicalDeviceCount is less than the number of physical devices available, at most pPhysicalDeviceCount structures will be written.
 * If pPhysicalDeviceCount is smaller than the number of physical devices available, VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the
 * available physical devices were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceCount,
	VkPhysicalDevice*                           pPhysicalDevices)
{
	assert(instance);

	//TODO is there a way to check if there's a gpu (and it's the rPi)?
	int gpuExists = access( "/dev/dri/card0", F_OK ) != -1;

	int numGPUs = gpuExists;

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
		//TODO no allocator, we probably shouldn't allocate
		pPhysicalDevices[c] = malloc(sizeof(_physicalDevice));
		assert(pPhysicalDevices[c]);
	}

	*pPhysicalDeviceCount = elementsWritten;

	if(elementsWritten < arraySize)
	{
		return VK_INCOMPLETE;
	}
	else
	{
		return VK_SUCCESS;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceQueueFamilyProperties
 * If pQueueFamilyProperties is NULL, then the number of queue families available is returned in pQueueFamilyPropertyCount.
 * Otherwise, pQueueFamilyPropertyCount must point to a variable set by the user to the number of elements in the pQueueFamilyProperties array,
 * and on return the variable is overwritten with the number of structures actually written to pQueueFamilyProperties. If pQueueFamilyPropertyCount
 * is less than the number of queue families available, at most pQueueFamilyPropertyCount structures will be written.
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(
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
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfaceSupportKHR
 * does this queue family support presentation to this surface?
 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(
	VkPhysicalDevice                            physicalDevice,
	uint32_t                                    queueFamilyIndex,
	VkSurfaceKHR                                surface,
	VkBool32*                                   pSupported)
{
	assert(pSupported);
	assert(surface);
	assert(physicalDevice);

	assert(queueFamilyIndex < numQueueFamilies);

	*pSupported = VK_TRUE; //TODO suuure for now, but we should verify if queue supports presenting to surface
	return VK_SUCCESS;
}

/*
 * Implementation of our RPI specific "extension"
 */
VkResult vkCreateRpiSurfaceKHR(
			  VkInstance                                  instance,
			  const VkRpiSurfaceCreateInfoKHR*            pCreateInfo,
			  const VkAllocationCallbacks*                pAllocator,
			  VkSurfaceKHR*                               pSurface)
{
	assert(pSurface);
	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	int ret = modeset_open("/dev/dri/card0"); assert(!ret);
	*pSurface = (VkSurfaceKHR)modeset_create();

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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	modeset_destroy((modeset_dev*)surface);
	modeset_close();
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
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(
	VkPhysicalDevice                            physicalDevice,
	const VkDeviceCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDevice*                                   pDevice)
{
	assert(physicalDevice);
	assert(pDevice);

	//TODO verify extensions, features

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	*pDevice = malloc(sizeof(_device));
	if(!pDevice)
	{
		return VK_ERROR_TOO_MANY_OBJECTS;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetDeviceQueue
 * vkGetDeviceQueue must only be used to get queues that were created with the flags parameter of VkDeviceQueueCreateInfo set to zero.
 * To get queues that were created with a non-zero flags parameter use vkGetDeviceQueue2.
 */
VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(
	VkDevice                                    device,
	uint32_t                                    queueFamilyIndex,
	uint32_t                                    queueIndex,
	VkQueue*                                    pQueue)
{
	assert(device);
	assert(pQueue);

	assert(queueFamilyIndex < numQueueFamilies);
	assert(queueIndex < _queueFamilyProperties[queueFamilyIndex].queueCount);

	*pQueue = &_queuesByFamily[queueFamilyIndex][queueIndex];
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateSemaphore
 * Semaphores are a synchronization primitive that can be used to insert a dependency between batches submitted to queues.
 * Semaphores have two states - signaled and unsignaled. The state of a semaphore can be signaled after execution of a batch of commands is completed.
 * A batch can wait for a semaphore to become signaled before it begins execution, and the semaphore is also unsignaled before the batch begins execution.
 * As with most objects in Vulkan, semaphores are an interface to internal data which is typically opaque to applications.
 * This internal data is referred to as a semaphore’s payload. However, in order to enable communication with agents outside of the current device,
 * it is necessary to be able to export that payload to a commonly understood format, and subsequently import from that format as well.
 * The internal data of a semaphore may include a reference to any resources and pending work associated with signal or unsignal operations performed on that semaphore object.
 * Mechanisms to import and export that internal data to and from semaphores are provided below.
 * These mechanisms indirectly enable applications to share semaphore state between two or more semaphores and other synchronization primitives across process and API boundaries.
 * When created, the semaphore is in the unsignaled state.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(
	VkDevice                                    device,
	const VkSemaphoreCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSemaphore*                                pSemaphore)
{
	assert(device);
	assert(pSemaphore);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//we'll probably just use an IOCTL to wait for a GPU sequence number to complete.
	*pSemaphore = -1;

	return VK_SUCCESS;
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
	pSurfaceCapabilities->currentExtent.width = ((modeset_dev*)surface)->bufs[0].width;
	pSurfaceCapabilities->currentExtent.height = ((modeset_dev*)surface)->bufs[0].height;
	pSurfaceCapabilities->minImageExtent.width = ((modeset_dev*)surface)->bufs[0].width; //TODO
	pSurfaceCapabilities->minImageExtent.height = ((modeset_dev*)surface)->bufs[0].height; //TODO
	pSurfaceCapabilities->maxImageExtent.width = ((modeset_dev*)surface)->bufs[0].width; //TODO
	pSurfaceCapabilities->maxImageExtent.height = ((modeset_dev*)surface)->bufs[0].height; //TODO
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
		//TODO
		pSurfaceFormats[c].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		pSurfaceFormats[c].format = VK_FORMAT_R8G8B8A8_UNORM;
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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	*pSwapchain = pCreateInfo->surface; //TODO

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

	const int numImages = 2;

	if(!pSwapchainImages)
	{
		*pSwapchainImageCount = numImages;
		return VK_SUCCESS;
	}

	int arraySize = *pSwapchainImageCount;
	int elementsWritten = min(numImages, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		//TODO
		pSwapchainImages[c] = c;
	}

	*pSwapchainImageCount = elementsWritten;

	if(elementsWritten < numImages)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffers-pools
 * Command pools are opaque objects that command buffer memory is allocated from, and which allow the implementation to amortize the
 * cost of resource creation across multiple command buffers. Command pools are externally synchronized, meaning that a command pool must
 * not be used concurrently in multiple threads. That includes use via recording commands on any command buffers allocated from the pool,
 * as well as operations that allocate, free, and reset command buffers or the pool itself.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(
	VkDevice                                    device,
	const VkCommandPoolCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkCommandPool*                              pCommandPool)
{
	assert(device);
	assert(pCreateInfo);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	*pCommandPool = 0; //TODO implement pool memory allocator

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffer-allocation
 * vkAllocateCommandBuffers can be used to create multiple command buffers. If the creation of any of those command buffers fails,
 * the implementation must destroy all successfully created command buffer objects from this command, set all entries of the pCommandBuffers array to NULL and return the error.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(
	VkDevice                                    device,
	const VkCommandBufferAllocateInfo*          pAllocateInfo,
	VkCommandBuffer*                            pCommandBuffers)
{
	assert(device);
	assert(pAllocateInfo);
	assert(pCommandBuffers);

	VkResult res = VK_SUCCESS;

	for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
	{
		pCommandBuffers[c] = malloc(sizeof(_commandBuffer));
		if(!pCommandBuffers[c])
		{
			res = VK_ERROR_OUT_OF_HOST_MEMORY; //TODO or VK_ERROR_OUT_OF_DEVICE_MEMORY?
		}
	}

	if(res != VK_SUCCESS)
	{
		for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
		{
			free(pCommandBuffers[c]);
			pCommandBuffers[c] = 0;
		}
	}

	return res;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBeginCommandBuffer
 */
VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(
	VkCommandBuffer                             commandBuffer,
	const VkCommandBufferBeginInfo*             pBeginInfo)
{
	assert(commandBuffer);
	assert(pBeginInfo);

	//TODO

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdPipelineBarrier
 * vkCmdPipelineBarrier is a synchronization command that inserts a dependency between commands submitted to the same queue, or between commands in the same subpass.
 * When vkCmdPipelineBarrier is submitted to a queue, it defines a memory dependency between commands that were submitted before it, and those submitted after it.
 * If vkCmdPipelineBarrier was recorded outside a render pass instance, the first synchronization scope includes all commands that occur earlier in submission order.
 * If vkCmdPipelineBarrier was recorded inside a render pass instance, the first synchronization scope includes only commands that occur earlier in submission order within the same subpass.
 * In either case, the first synchronization scope is limited to operations on the pipeline stages determined by the source stage mask specified by srcStageMask.
 *
 * If vkCmdPipelineBarrier was recorded outside a render pass instance, the second synchronization scope includes all commands that occur later in submission order.
 * If vkCmdPipelineBarrier was recorded inside a render pass instance, the second synchronization scope includes only commands that occur later in submission order within the same subpass.
 * In either case, the second synchronization scope is limited to operations on the pipeline stages determined by the destination stage mask specified by dstStageMask.
 *
 * The first access scope is limited to access in the pipeline stages determined by the source stage mask specified by srcStageMask.
 * Within that, the first access scope only includes the first access scopes defined by elements of the pMemoryBarriers,
 * pBufferMemoryBarriers and pImageMemoryBarriers arrays, which each define a set of memory barriers. If no memory barriers are specified,
 * then the first access scope includes no accesses.
 *
 * The second access scope is limited to access in the pipeline stages determined by the destination stage mask specified by dstStageMask.
 * Within that, the second access scope only includes the second access scopes defined by elements of the pMemoryBarriers, pBufferMemoryBarriers and pImageMemoryBarriers arrays,
 * which each define a set of memory barriers. If no memory barriers are specified, then the second access scope includes no accesses.
 *
 * If dependencyFlags includes VK_DEPENDENCY_BY_REGION_BIT, then any dependency between framebuffer-space pipeline stages is framebuffer-local - otherwise it is framebuffer-global.
 */
VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlags                        srcStageMask,
	VkPipelineStageFlags                        dstStageMask,
	VkDependencyFlags                           dependencyFlags,
	uint32_t                                    memoryBarrierCount,
	const VkMemoryBarrier*                      pMemoryBarriers,
	uint32_t                                    bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
	uint32_t                                    imageMemoryBarrierCount,
	const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
	assert(commandBuffer);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearColorImage
 * Color and depth/stencil images can be cleared outside a render pass instance using vkCmdClearColorImage or vkCmdClearDepthStencilImage, respectively.
 * These commands are only allowed outside of a render pass instance.
 */
VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearColorValue*                    pColor,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges)
{
	assert(commandBuffer);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEndCommandBuffer
 * If there was an error during recording, the application will be notified by an unsuccessful return code returned by vkEndCommandBuffer.
 * If the application wishes to further use the command buffer, the command buffer must be reset. The command buffer must have been in the recording state,
 * and is moved to the executable state.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(
	VkCommandBuffer                             commandBuffer)
{
	assert(commandBuffer);

	//TODO

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

	//TODO

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkQueueSubmit
 * vkQueueSubmit is a queue submission command, with each batch defined by an element of pSubmits as an instance of the VkSubmitInfo structure.
 * Batches begin execution in the order they appear in pSubmits, but may complete out of order.
 * Fence and semaphore operations submitted with vkQueueSubmit have additional ordering constraints compared to other submission commands,
 * with dependencies involving previous and subsequent queue operations. Information about these additional constraints can be found in the semaphore and
 * fence sections of the synchronization chapter.
 * Details on the interaction of pWaitDstStageMask with synchronization are described in the semaphore wait operation section of the synchronization chapter.
 * The order that batches appear in pSubmits is used to determine submission order, and thus all the implicit ordering guarantees that respect it.
 * Other than these implicit ordering guarantees and any explicit synchronization primitives, these batches may overlap or otherwise execute out of order.
 * If any command buffer submitted to this queue is in the executable state, it is moved to the pending state. Once execution of all submissions of a command buffer complete,
 * it moves from the pending state, back to the executable state. If a command buffer was recorded with the VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT flag,
 * it instead moves back to the invalid state.
 * If vkQueueSubmit fails, it may return VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY.
 * If it does, the implementation must ensure that the state and contents of any resources or synchronization primitives referenced by the submitted command buffers and any semaphores
 * referenced by pSubmits is unaffected by the call or its failure. If vkQueueSubmit fails in such a way that the implementation is unable to make that guarantee,
 * the implementation must return VK_ERROR_DEVICE_LOST. See Lost Device.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(
	VkQueue                                     queue,
	uint32_t                                    submitCount,
	const VkSubmitInfo*                         pSubmits,
	VkFence                                     fence)
{
	assert(queue);

	//TODO

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

	for(int c = 0; c < pPresentInfo->swapchainCount; ++c)
	{
		//TODO
		modeset_swapbuffer((modeset_dev*)pPresentInfo->pSwapchains[c], pPresentInfo->pImageIndices[c]);
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDeviceWaitIdle
 * vkDeviceWaitIdle is equivalent to calling vkQueueWaitIdle for all queues owned by device.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(
		VkDevice									device)
{
	assert(device);

	//TODO
	//possibly wait on ioctl

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkFreeCommandBuffers
 * Any primary command buffer that is in the recording or executable state and has any element of pCommandBuffers recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		uint32_t                                    commandBufferCount,
		const VkCommandBuffer*                      pCommandBuffers)
{
	assert(device);
	//assert(commandPool); //TODO
	assert(pCommandBuffers);

	for(int c = 0; c < commandBufferCount; ++c)
	{
		free(pCommandBuffers[c]); //TODO
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyCommandPool
 * When a pool is destroyed, all command buffers allocated from the pool are freed.
 * Any primary command buffer allocated from another VkCommandPool that is in the recording or executable state and has a secondary command buffer
 * allocated from commandPool recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	//assert(commandPool); //TODO

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySemaphore
 */
VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(
	VkDevice                                    device,
	VkSemaphore                                 semaphore,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(semaphore);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO
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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyDevice
 * To ensure that no work is active on the device, vkDeviceWaitIdle can be used to gate the destruction of the device.
 * Prior to destroying a device, an application is responsible for destroying/freeing any Vulkan objects that were created using that device as the
 * first parameter of the corresponding vkCreate* or vkAllocate* command
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(
	VkDevice                                    device,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyInstance
 *
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(
	VkInstance                                  instance,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(instance);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO
}

