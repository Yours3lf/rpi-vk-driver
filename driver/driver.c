#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
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
 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
	VkPhysicalDevice                            physicalDevice,
	VkSurfaceKHR                                surface,
	VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(
		VkDevice									device)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		uint32_t                                    commandBufferCount,
		const VkCommandBuffer*                      pCommandBuffers)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(
	VkDevice                                    device,
	VkSemaphore                                 semaphore,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(
	VkDevice                                    device,
	VkSwapchainKHR                              swapchain,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(
	VkDevice                                    device,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(
	VkInstance                                  instance,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(
	VkQueue                                     queue,
	const VkPresentInfoKHR*                     pPresentInfo)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(
	VkQueue                                     queue,
	uint32_t                                    submitCount,
	const VkSubmitInfo*                         pSubmits,
	VkFence                                     fence)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(
	VkDevice                                    device,
	VkSwapchainKHR                              swapchain,
	uint64_t                                    timeout,
	VkSemaphore                                 semaphore,
	VkFence                                     fence,
	uint32_t*                                   pImageIndex)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(
	VkCommandBuffer                             commandBuffer)
{
	return VK_SUCCESS;
}

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

}

VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearColorValue*                    pColor,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges)
{

}

VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(
	VkCommandBuffer                             commandBuffer,
	const VkCommandBufferBeginInfo*             pBeginInfo)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(
	VkDevice                                    device,
	const VkCommandBufferAllocateInfo*          pAllocateInfo,
	VkCommandBuffer*                            pCommandBuffers)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(
	VkDevice                                    device,
	const VkCommandPoolCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkCommandPool*                              pCommandPool)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(
	VkDevice                                    device,
	VkSwapchainKHR                              swapchain,
	uint32_t*                                   pSwapchainImageCount,
	VkImage*                                    pSwapchainImages)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(
	VkDevice                                    device,
	const VkSwapchainCreateInfoKHR*             pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSwapchainKHR*                             pSwapchain)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(
	VkPhysicalDevice                            physicalDevice,
	VkSurfaceKHR                                surface,
	uint32_t*                                   pPresentModeCount,
	VkPresentModeKHR*                           pPresentModes)
{
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(
	VkPhysicalDevice                            physicalDevice,
	VkSurfaceKHR                                surface,
	uint32_t*                                   pSurfaceFormatCount,
	VkSurfaceFormatKHR*                         pSurfaceFormats)
{
	return VK_SUCCESS;
}
