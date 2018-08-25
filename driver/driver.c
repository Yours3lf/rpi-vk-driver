#include <stdio.h>
#include "CustomAssert.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include <vulkan/vulkan.h>
#include "vkExt.h"

#include "modeset.h"
#include "kernelInterface.h"
#include "ControlListUtil.h"

#include "AlignedAllocator.h"
#include "PoolAllocator.h"
#include "ConsecutivePoolAllocator.h"
#include "LinearAllocator.h"

#include "kernel/vc4_packet.h"
#include "../brcm/cle/v3d_decoder.h"
#include "../brcm/clif/clif_dump.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

#include "vkCaps.h"

typedef struct VkPhysicalDevice_T
{
	//hardware id?
	int dummy;
} _physicalDevice;

typedef struct VkQueue_T
{
	uint64_t lastEmitSeqno;
} _queue;

typedef struct VkCommandPool_T
{
	PoolAllocator pa;
	ConsecutivePoolAllocator cpa;
	uint32_t queueFamilyIndex;
} _commandPool;

typedef enum commandBufferState
{
	CMDBUF_STATE_INITIAL = 0,
	CMDBUF_STATE_RECORDING,
	CMDBUF_STATE_EXECUTABLE,
	CMDBUF_STATE_PENDING,
	CMDBUF_STATE_INVALID,
	CMDBUF_STATE_LAST
} commandBufferState;

typedef struct VkCommandBuffer_T
{
	//Recorded commands include commands to bind pipelines and descriptor sets to the command buffer, commands to modify dynamic state, commands to draw (for graphics rendering),
	//commands to dispatch (for compute), commands to execute secondary command buffers (for primary command buffers only), commands to copy buffers and images, and other commands

	struct drm_vc4_submit_cl submitCl;

	ControlList binCl;
	ControlList shaderRecCl;
	uint32_t shaderRecCount;
	ControlList uniformsCl;
	ControlList handlesCl;
	commandBufferState state;
	VkCommandBufferUsageFlags usageFlags;
	_commandPool* cp;
} _commandBuffer;

typedef struct VkInstance_T
{
	//supposedly this should contain all the enabled layers?
	int enabledExtensions[numInstanceExtensions];
	int numEnabledExtensions;
	_physicalDevice dev;
	int chipVersion;
	int hasTiling;
	int hasControlFlow;
	int hasEtc1;
	int hasThreadedFs;
	int hasMadvise;
} _instance;

typedef struct VkDevice_T
{
	int enabledExtensions[numDeviceExtensions];
	int numEnabledExtensions;
	VkPhysicalDeviceFeatures enabledFeatures;
	_physicalDevice* dev;
	_queue* queues[numQueueFamilies];
	int numQueues[numQueueFamilies];
} _device;

typedef struct VkSwapchain_T
{
	_image* images;
	uint32_t numImages;
	uint32_t backbufferIdx;
	VkSurfaceKHR surface;
} _swapchain;

void clFit(VkCommandBuffer cb, ControlList* cl, uint32_t commandSize)
{
	if(!clHasEnoughSpace(cl, commandSize))
	{
		uint32_t currSize = clSize(cl);
		cl->buffer = consecutivePoolReAllocate(&cb->cp->cpa, cl->buffer, cl->numBlocks); assert(cl->buffer);
		cl->nextFreeByte = cl->buffer + currSize;
	}
}

void clDump(void* cl, uint32_t size)
{
		struct v3d_device_info devinfo = {
				/* While the driver supports V3D 2.1 and 2.6, we haven't split
				 * off a 2.6 XML yet (there are a couple of fields different
				 * in render target formatting)
				 */
				.ver = 21,
		};
		struct v3d_spec* spec = v3d_spec_load(&devinfo);

		struct clif_dump *clif = clif_dump_init(&devinfo, stderr, true);

		uint32_t offset = 0, hw_offset = 0;
		uint8_t *p = cl;

		while (offset < size) {
				struct v3d_group *inst = v3d_spec_find_instruction(spec, p);
				uint8_t header = *p;
				uint32_t length;

				if (inst == NULL) {
						printf("0x%08x 0x%08x: Unknown packet 0x%02x (%d)!\n",
								offset, hw_offset, header, header);
						return;
				}

				length = v3d_group_get_length(inst);

				printf("0x%08x 0x%08x: 0x%02x %s\n",
						offset, hw_offset, header, v3d_group_get_name(inst));

				v3d_print_group(clif, inst, offset, p);

				switch (header) {
				case VC4_PACKET_HALT:
				case VC4_PACKET_STORE_MS_TILE_BUFFER_AND_EOF:
						return;
				default:
						break;
				}

				offset += length;
				if (header != VC4_PACKET_GEM_HANDLES)
						hw_offset += length;
				p += length;
		}

		clif_dump_destroy(clif);
}

//Textures in T format:
//formed out of 4KB tiles, which have 1KB subtiles (see page 105 in VC4 arch guide)
//1KB subtiles have 512b microtiles.
//Width/height of the 512b microtiles is the following:
// 64bpp: 2x4
// 32bpp: 4x4
// 16bpp: 8x4
// 8bpp:  8x8
// 4bpp:  16x8
// 1bpp:  32x16
//Therefore width/height of 1KB subtiles is the following:
// 64bpp: 8x16
// 32bpp: 16x16
// 16bpp: 32x16
// 8bpp:  32x32
// 4bpp:  64x32
// 1bpp:  128x64
//Finally width/height of the 4KB tiles:
// 64bpp: 16x32
// 32bpp: 32x32
// 16bpp: 64x32
// 8bpp:  64x64
// 4bpp:  128x64
// 1bpp:  256x128
void getPaddedTextureDimensionsT(uint32_t width, uint32_t height, uint32_t bpp, uint32_t* paddedWidth, uint32_t* paddedHeight)
{
	assert(paddedWidth);
	assert(paddedHeight);
	uint32_t tileW = 0;
	uint32_t tileH = 0;

	switch(bpp)
	{
	case 64:
	{
		tileW = 16;
		tileH = 32;
		break;
	}
	case 32:
	{
		tileW = 32;
		tileH = 32;
		break;
	}
	case 16:
	{
		tileW = 64;
		tileH = 32;
		break;
	}
	case 8:
	{
		tileW = 64;
		tileH = 64;
		break;
	}
	case 4:
	{
		tileW = 128;
		tileH = 64;
		break;
	}
	case 1:
	{
		tileW = 256;
		tileH = 128;
		break;
	}
	default:
	{
		assert(0); //unsupported
	}
	}

	*paddedWidth = ((tileW - (width % tileW)) % tileW) + width;
	*paddedHeight = ((tileH - (height % tileH)) % tileH) + height;
}

uint32_t getFormatBpp(VkFormat f)
{
	switch(f)
	{
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 64;
	case VK_FORMAT_R8G8B8_UNORM: //padded to 32
	case VK_FORMAT_R8G8B8A8_UNORM:
		return 32;
		return 32;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16_SINT:
		return 16;
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SINT:
		return 8;
	default:
		assert(0);
		return 0;
	}
}

void createImageBO(_image* i)
{
	assert(i);
	assert(i->format);
	assert(i->width);
	assert(i->height);

	uint32_t bpp = getFormatBpp(i->format);
	uint32_t pixelSizeBytes = bpp / 8;
	uint32_t nonPaddedSize = i->width * i->height * pixelSizeBytes;
	i->paddedWidth = i->width;
	i->paddedHeight = i->height;

	//need to pad to T format, as HW automatically chooses that
	if(nonPaddedSize > 4096)
	{
		getPaddedTextureDimensionsT(i->width, i->height, bpp, &i->paddedWidth, &i->paddedHeight);
	}

	i->size = i->paddedWidth * i->paddedHeight * pixelSizeBytes;
	i->stride = i->paddedWidth * pixelSizeBytes;
	i->handle = vc4_bo_alloc(controlFd, i->size, "swapchain image"); assert(i->handle);

	//set tiling to T if size > 4KB
	if(nonPaddedSize > 4096)
	{
		int ret = vc4_bo_set_tiling(controlFd, i->handle, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED); assert(ret);
		i->tiling = VC4_TILING_FORMAT_T;
	}
	else
	{
		int ret = vc4_bo_set_tiling(controlFd, i->handle, DRM_FORMAT_MOD_LINEAR); assert(ret);
		i->tiling = VC4_TILING_FORMAT_LT;
	}
}

/*static inline void util_pack_color(const float rgba[4], enum pipe_format format, union util_color *uc)
{
   ubyte r = 0;
   ubyte g = 0;
   ubyte b = 0;
   ubyte a = 0;

   if (util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0) <= 8) {
	  r = float_to_ubyte(rgba[0]);
	  g = float_to_ubyte(rgba[1]);
	  b = float_to_ubyte(rgba[2]);
	  a = float_to_ubyte(rgba[3]);
   }

   switch (format) {
   case PIPE_FORMAT_ABGR8888_UNORM:
	  {
		 uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | a;
	  }
	  return;
   case PIPE_FORMAT_XBGR8888_UNORM:
	  {
		 uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | 0xff;
	  }
	  return;
   case PIPE_FORMAT_BGRA8888_UNORM:
	  {
		 uc->ui[0] = (a << 24) | (r << 16) | (g << 8) | b;
	  }
	  return;
   case PIPE_FORMAT_BGRX8888_UNORM:
	  {
		 uc->ui[0] = (0xffu << 24) | (r << 16) | (g << 8) | b;
	  }
	  return;
   case PIPE_FORMAT_ARGB8888_UNORM:
	  {
		 uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | a;
	  }
	  return;
   case PIPE_FORMAT_XRGB8888_UNORM:
	  {
		 uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | 0xff;
	  }
	  return;
   case PIPE_FORMAT_B5G6R5_UNORM:
	  {
		 uc->us = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
	  {
		 uc->us = ((0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
	  {
		 uc->us = ((a & 0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
	  {
		 uc->us = ((a & 0xf0) << 8) | ((r & 0xf0) << 4) | ((g & 0xf0) << 0) | (b >> 4);
	  }
	  return;
   case PIPE_FORMAT_A8_UNORM:
	  {
		 uc->ub = a;
	  }
	  return;
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
	  {
		 uc->ub = r;
	  }
	  return;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
	  {
		 uc->f[0] = rgba[0];
		 uc->f[1] = rgba[1];
		 uc->f[2] = rgba[2];
		 uc->f[3] = rgba[3];
	  }
	  return;
   case PIPE_FORMAT_R32G32B32_FLOAT:
	  {
		 uc->f[0] = rgba[0];
		 uc->f[1] = rgba[1];
		 uc->f[2] = rgba[2];
	  }
	  return;

   default:
	  util_format_write_4f(format, rgba, 0, uc, 0, 0, 0, 1, 1);
   }
}*/

uint32_t packVec4IntoABGR8(const float rgba[4])
{
	uint8_t r, g, b, a;
	r = rgba[0] * 255.0;
	g = rgba[1] * 255.0;
	b = rgba[2] * 255.0;
	a = rgba[3] * 255.0;

	uint32_t res = 0 |
			(a << 24) |
			(b << 16) |
			(g << 8) |
			(r << 0);

	return res;
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
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(!pLayerName); //TODO layers ignored for now
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numInstanceExtensions;
		return VK_INCOMPLETE;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numInstanceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = instanceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	return VK_SUCCESS;
}

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
	assert(pInstance);
	assert(pCreateInfo);

	*pInstance = malloc(sizeof(_instance));

	if(!*pInstance)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	(*pInstance)->numEnabledExtensions = 0;

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO: possibly we need to load layers here
	//and store them in pInstance
	assert(pCreateInfo->enabledLayerCount == 0);

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
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//TODO ignored for now
	//pCreateInfo->pApplicationInfo

	int ret = openIoctl(); assert(!ret);

	(*pInstance)->chipVersion = vc4_get_chip_info(controlFd);
	(*pInstance)->hasTiling = vc4_test_tiling(controlFd);

	(*pInstance)->hasControlFlow = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
	(*pInstance)->hasEtc1 = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_ETC1);
	(*pInstance)->hasThreadedFs = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
	(*pInstance)->hasMadvise = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_MADVISE);

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
		pPhysicalDevices[c] = &instance->dev;
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
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceProperties
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(
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

	pProperties->apiVersion = VK_MAKE_VERSION(1,1,0);
	pProperties->driverVersion = 1; //we'll simply call this v1
	pProperties->vendorID = 0x14E4; //Broadcom
	pProperties->deviceID = 0; //TODO dunno?
	pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
	strcpy(pProperties->deviceName, "VideoCore IV HW");
	//pProperties->pipelineCacheUUID
	pProperties->limits = _limits;
	pProperties->sparseProperties = sparseProps;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceFeatures
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(
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
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(
		VkPhysicalDevice                            physicalDevice,
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(physicalDevice);
	assert(!pLayerName); //layers ignored for now
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numDeviceExtensions;
		return VK_INCOMPLETE;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numDeviceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = deviceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	return VK_SUCCESS;
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

	//TODO if we plan to support headless rendering, there should be 2 families
	//one using /dev/dri/card0 which has modesetting
	//other using /dev/dri/renderD128 which does not support modesetting, this would say false here
	*pSupported = VK_TRUE;
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
	assert(instance);
	//assert(pCreateInfo); //ignored for now
	assert(pSurface);
	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	modeset_destroy(controlFd, (modeset_dev*)surface);
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
	assert(pCreateInfo);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	*pDevice = malloc(sizeof(_device));
	if(!pDevice)
	{
		return VK_ERROR_TOO_MANY_OBJECTS;
	}

	(*pDevice)->dev = physicalDevice;

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findDeviceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pDevice)->enabledExtensions[(*pDevice)->numEnabledExtensions] = findres;
			(*pDevice)->numEnabledExtensions++;
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

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
			(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex] = malloc(sizeof(_queue)*pCreateInfo->pQueueCreateInfos[c].queueCount);

			if(!(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			for(int d = 0; d < pCreateInfo->pQueueCreateInfos[c].queueCount; ++d)
			{
				(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex][d].lastEmitSeqno = 0;
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
VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(
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
	sem_t* s = malloc(sizeof(sem_t));
	if(!s)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}
	sem_init(s, 0, 0); //create semaphore unsignalled, shared between threads

	*pSemaphore = (VkSemaphore)s;

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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	*pSwapchain = malloc(sizeof(_swapchain));
	if(!*pSwapchain)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	_swapchain* s = *pSwapchain;

	//TODO flags, layers, queue sharing, pretransform, composite alpha, present mode..., clipped, oldswapchain
	//TODO external sync on surface, oldswapchain

	s->images = malloc(sizeof(_image) * pCreateInfo->minImageCount);
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
			s->images[c].queueFamiliesWithAccess = malloc(sizeof(uint32_t)*s->images[c].numQueueFamiliesWithAccess);
			memcpy(s->images[c].queueFamiliesWithAccess, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t)*s->images[c].numQueueFamiliesWithAccess);
		}
		s->images[c].preTransformMode = pCreateInfo->preTransform;
		s->images[c].compositeAlpha = pCreateInfo->compositeAlpha;
		s->images[c].presentMode = pCreateInfo->presentMode;
		s->images[c].clipped = pCreateInfo->clipped;

		createImageBO(&s->images[c]);
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

	//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	//specifies that command buffers allocated from the pool will be short-lived, meaning that they will be reset or freed in a relatively short timeframe.
	//This flag may be used by the implementation to control memory allocation behavior within the pool.
	//--> definitely use pool allocator

	//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	//allows any command buffer allocated from a pool to be individually reset to the initial state; either by calling vkResetCommandBuffer, or via the implicit reset when calling vkBeginCommandBuffer.
	//If this flag is not set on a pool, then vkResetCommandBuffer must not be called for any command buffer allocated from that pool.

	//TODO pool family ignored for now

	_commandPool* cp = malloc(sizeof(_commandPool));

	if(!cp)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	cp->queueFamilyIndex = pCreateInfo->queueFamilyIndex;

	//initial number of command buffers to hold
	int numCommandBufs = 100;
	int controlListSize = ARM_PAGE_SIZE * 100;

	//if(pCreateInfo->flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	{
		//use pool allocator
		void* pamem = malloc(numCommandBufs * sizeof(_commandBuffer));
		if(!pamem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->pa = createPoolAllocator(pamem, sizeof(_commandBuffer), numCommandBufs * sizeof(_commandBuffer));

		void* cpamem = malloc(controlListSize);
		if(!cpamem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->cpa = createConsecutivePoolAllocator(cpamem, ARM_PAGE_SIZE, controlListSize);
	}

	*pCommandPool = (VkCommandPool)cp;

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

	_commandPool* cp = (_commandPool*)pAllocateInfo->commandPool;

	//if(cp->usePoolAllocator)
	{
		for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
		{
			pCommandBuffers[c] = poolAllocate(&cp->pa);

			if(!pCommandBuffers[c])
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			pCommandBuffers[c]->shaderRecCount = 0;
			pCommandBuffers[c]->usageFlags = 0;
			pCommandBuffers[c]->state = CMDBUF_STATE_INITIAL;
			pCommandBuffers[c]->cp = cp;
			clInit(&pCommandBuffers[c]->binCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->handlesCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->shaderRecCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->uniformsCl, consecutivePoolAllocate(&cp->cpa, 1));

			if(!pCommandBuffers[c]->binCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->handlesCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->shaderRecCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->uniformsCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}
		}
	}

	if(res != VK_SUCCESS)
	{
		//if(cp->usePoolAllocator)
		{
			for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
			{
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->binCl, pCommandBuffers[c]->binCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->handlesCl, pCommandBuffers[c]->binCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->shaderRecCl, pCommandBuffers[c]->binCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->uniformsCl, pCommandBuffers[c]->binCl.numBlocks);
				poolFree(&cp->pa, pCommandBuffers[c]);
				pCommandBuffers[c] = 0;
			}
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

	//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	//specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission.

	//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
	//specifies that a secondary command buffer is considered to be entirely inside a render pass. If this is a primary command buffer, then this bit is ignored

	//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
	//specifies that a command buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary command buffers

	//When a command buffer begins recording, all state in that command buffer is undefined

	struct drm_vc4_submit_cl submitCl =
	{
		.color_read.hindex = ~0,
		.zs_read.hindex = ~0,
		.color_write.hindex = ~0,
		.msaa_color_write.hindex = ~0,
		.zs_write.hindex = ~0,
		.msaa_zs_write.hindex = ~0,
	};

	commandBuffer->usageFlags = pBeginInfo->flags;
	commandBuffer->shaderRecCount = 0;
	commandBuffer->state = CMDBUF_STATE_RECORDING;
	commandBuffer->submitCl = submitCl;


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
	assert(image);
	assert(pColor);

	//TODO this should only flag an image for clearing. This can only be called outside a renderpass
	//actual clearing would only happen:
	// -if image is rendered to (insert clear before first draw call)
	// -if the image is bound for sampling (submit a CL with a clear)
	// -if the command buffer is closed without any rendering (insert clear)
	// -etc.
	//we shouldn't clear an image if noone uses it

	//TODO ranges support

	assert(imageLayout == VK_IMAGE_LAYOUT_GENERAL ||
		   imageLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR ||
		   imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	assert(commandBuffer->state	 == CMDBUF_STATE_RECORDING);
	assert(_queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT || _queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT);

	//TODO externally sync cmdbuf, cmdpool

	_image* i = image;

	assert(i->usageBits & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	clFit(commandBuffer, &commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
	clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
										 0, 0, 0, 0,
										 getFormatBpp(i->format) == 64, //64 bit color mode
										 i->samples > 1, //msaa
										 i->width, i->height, 0, 0, 0);

	//START_TILE_BINNING resets the statechange counters in the hardware,
	//which are what is used when a primitive is binned to a tile to
	//figure out what new state packets need to be written to that tile's
	//command list.
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
	clInsertStartTileBinning(&commandBuffer->binCl);

	//Reset the current compressed primitives format.  This gets modified
	//by VC4_PACKET_GL_INDEXED_PRIMITIVE and
	//VC4_PACKET_GL_ARRAY_PRIMITIVE, so it needs to be reset at the start
	//of every tile.
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_PRIMITIVE_LIST_FORMAT_length);
	clInsertPrimitiveListFormat(&commandBuffer->binCl,
								1, //16 bit
								2); //tris

	clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, i->handle);
	commandBuffer->submitCl.color_write.hindex = idx;
	commandBuffer->submitCl.color_write.offset = 0;
	commandBuffer->submitCl.color_write.flags = 0;
	//TODO format
	commandBuffer->submitCl.color_write.bits =
			VC4_SET_FIELD(VC4_RENDER_CONFIG_FORMAT_RGBA8888, VC4_RENDER_CONFIG_FORMAT) |
			VC4_SET_FIELD(i->tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);

	//TODO msaa?

	commandBuffer->submitCl.clear_color[0] =
			commandBuffer->submitCl.clear_color[1] = packVec4IntoABGR8(pColor->float32);
	//TODO ranges
	commandBuffer->submitCl.min_x_tile = 0;
	commandBuffer->submitCl.min_y_tile = 0;

	uint32_t tileSizeW = 64;
	uint32_t tileSizeH = 64;

	if(i->samples > 1)
	{
		tileSizeW >>= 1;
		tileSizeH >>= 1;
	}

	if(getFormatBpp(i->format) == 64)
	{
		tileSizeH >>= 1;
	}

	uint32_t widthInTiles = divRoundUp(i->width, tileSizeW);
	uint32_t heightInTiles = divRoundUp(i->height, tileSizeH);

	commandBuffer->submitCl.max_x_tile = widthInTiles - 1;
	commandBuffer->submitCl.max_y_tile = heightInTiles - 1;
	commandBuffer->submitCl.width = i->width;
	commandBuffer->submitCl.height = i->height;
	commandBuffer->submitCl.flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;
	commandBuffer->submitCl.clear_z = 0; //TODO
	commandBuffer->submitCl.clear_s = 0;

	//TODO primitive list format must be followed by shader state
	//clFit(commandBuffer, &commandBuffer->binCl, V3D21_GL_SHADER_STATE_length);
	//clInsertShaderState(&commandBuffer->binCl, 0, 0, 0);

	//clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIP_WINDOW_length);
	//clInsertClipWindow(&commandBuffer->binCl, i->width, i->height, 0, 0); //TODO should this be configurable?

	/*
	//TODO
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CONFIGURATION_BITS_length);
	clInsertConfigurationBits(&commandBuffer->binCl,
							  0,
							  0,
							  0,
							  V3D_COMPARE_FUNC_ALWAYS,
							  0,
							  0,
							  0,
							  0,
							  0,
							  0,
							  0,
							  1,
							  1);

	//TODO
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_DEPTH_OFFSET_length);
	clInsertDepthOffset(&commandBuffer->binCl, 0, 0);

	clFit(commandBuffer, &commandBuffer->binCl, V3D21_POINT_SIZE_length);
	clInsertPointSize(&commandBuffer->binCl, 1.0f);

	clFit(commandBuffer, &commandBuffer->binCl, V3D21_LINE_WIDTH_length);
	clInsertLineWidth(&commandBuffer->binCl, 1.0f);

	//TODO
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_XY_SCALING_length);
	clInsertClipperXYScaling(&commandBuffer->binCl, 1.0f, 1.0f);

	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_Z_SCALE_AND_OFFSET_length);
	clInsertClipperZScaleOffset(&commandBuffer->binCl, 0.0f, 1.0f);

	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VIEWPORT_OFFSET_length);
	clInsertViewPortOffset(&commandBuffer->binCl, 0, 0);

	//TODO
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_FLAT_SHADE_FLAGS_length);
	clInsertFlatShadeFlags(&commandBuffer->binCl, 0);

	//TODO I suppose this should be a submit itself?
	*/
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

	//Increment the semaphore indicating that binning is done and
	//unblocking the render thread.  Note that this doesn't act
	//until the FLUSH completes.
	//The FLUSH caps all of our bin lists with a
	//VC4_PACKET_RETURN.
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_INCREMENT_SEMAPHORE_length);
	clInsertIncrementSemaphore(&commandBuffer->binCl);
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_FLUSH_length);
	clInsertFlush(&commandBuffer->binCl);

	commandBuffer->state = CMDBUF_STATE_EXECUTABLE;

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

	for(int c = 0; c < pSubmits->waitSemaphoreCount; ++c)
	{
		sem_wait((sem_t*)pSubmits->pWaitSemaphores[c]);
	}

	//TODO: deal with pSubmits->pWaitDstStageMask

	//TODO wait for fence??

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		if(pSubmits->pCommandBuffers[c]->state == CMDBUF_STATE_EXECUTABLE)
		{
			pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_PENDING;
		}
	}

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		VkCommandBuffer cmdbuf = pSubmits->pCommandBuffers[c];

		cmdbuf->submitCl.bo_handles = cmdbuf->handlesCl.buffer;
		cmdbuf->submitCl.bo_handle_count = clSize(&cmdbuf->handlesCl) / 4;
		cmdbuf->submitCl.bin_cl = cmdbuf->binCl.buffer;
		cmdbuf->submitCl.bin_cl_size = clSize(&cmdbuf->binCl);
		cmdbuf->submitCl.shader_rec = cmdbuf->shaderRecCl.buffer;
		cmdbuf->submitCl.shader_rec_size = clSize(&cmdbuf->shaderRecCl);
		cmdbuf->submitCl.shader_rec_count = cmdbuf->shaderRecCount;
		cmdbuf->submitCl.uniforms = cmdbuf->uniformsCl.buffer;
		cmdbuf->submitCl.uniforms_size = clSize(&cmdbuf->uniformsCl);

		printf("BCL:\n");
		clDump(cmdbuf->submitCl.bin_cl, cmdbuf->submitCl.bin_cl_size);
		printf("BO handles: ");
		for(int d = 0; d < cmdbuf->submitCl.bo_handle_count; ++d)
		{
			printf("%u ", *((uint32_t*)(cmdbuf->submitCl.bo_handles)+d));
		}
		printf("\nwidth height: %u, %u\n", cmdbuf->submitCl.width, cmdbuf->submitCl.height);
		printf("tile min/max: %u,%u %u,%u\n", cmdbuf->submitCl.min_x_tile, cmdbuf->submitCl.min_y_tile, cmdbuf->submitCl.max_x_tile, cmdbuf->submitCl.max_y_tile);
		printf("color read surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.color_read.hindex, cmdbuf->submitCl.color_read.offset, cmdbuf->submitCl.color_read.bits, cmdbuf->submitCl.color_read.flags);
		printf("color write surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.color_write.hindex, cmdbuf->submitCl.color_write.offset, cmdbuf->submitCl.color_write.bits, cmdbuf->submitCl.color_write.flags);
		printf("zs read surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.zs_read.hindex, cmdbuf->submitCl.zs_read.offset, cmdbuf->submitCl.zs_read.bits, cmdbuf->submitCl.zs_read.flags);
		printf("zs write surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.zs_write.hindex, cmdbuf->submitCl.zs_write.offset, cmdbuf->submitCl.zs_write.bits, cmdbuf->submitCl.zs_write.flags);
		printf("msaa color write surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.msaa_color_write.hindex, cmdbuf->submitCl.msaa_color_write.offset, cmdbuf->submitCl.msaa_color_write.bits, cmdbuf->submitCl.msaa_color_write.flags);
		printf("msaa zs write surf: hindex, offset, bits, flags %u %u %u %u\n", cmdbuf->submitCl.msaa_zs_write.hindex, cmdbuf->submitCl.msaa_zs_write.offset, cmdbuf->submitCl.msaa_zs_write.bits, cmdbuf->submitCl.msaa_zs_write.flags);
		printf("clear color packed rgba %u %u\n", cmdbuf->submitCl.clear_color[0], cmdbuf->submitCl.clear_color[1]);
		printf("clear z %u\n", cmdbuf->submitCl.clear_z);
		printf("clear s %u\n", cmdbuf->submitCl.clear_s);
		printf("flags %u\n", cmdbuf->submitCl.flags);


		//submit ioctl
		static uint64_t lastFinishedSeqno = 0;
		vc4_cl_submit(controlFd, &cmdbuf->submitCl, &queue->lastEmitSeqno, &lastFinishedSeqno);
	}

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		if(pSubmits->pCommandBuffers[c]->state == CMDBUF_STATE_PENDING)
		{
			if(pSubmits->pCommandBuffers[c]->usageFlags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
			{
				pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_INVALID;
			}
			else
			{
				pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_EXECUTABLE;
			}
		}
	}

	for(int c = 0; c < pSubmits->signalSemaphoreCount; ++c)
	{
		sem_post((sem_t*)pSubmits->pSignalSemaphores[c]);
	}

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
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDeviceWaitIdle
 * vkDeviceWaitIdle is equivalent to calling vkQueueWaitIdle for all queues owned by device.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(
		VkDevice									device)
{
	assert(device);

	for(int c = 0; c < numQueueFamilies; ++c)
	{
		for(int d = 0; d < device->numQueues[c]; ++d)
		{
			uint64_t lastFinishedSeqno;
			vc4_seqno_wait(controlFd, &lastFinishedSeqno, device->queues[c][d].lastEmitSeqno, WAIT_TIMEOUT_INFINITE);
		}
	}

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
	assert(commandPool);
	assert(pCommandBuffers);

	_commandPool* cp = (_commandPool*)commandPool;

	for(int c = 0; c < commandBufferCount; ++c)
	{
		//if(cp->usePoolAllocator)
		{
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->binCl, pCommandBuffers[c]->binCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->handlesCl, pCommandBuffers[c]->binCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->shaderRecCl, pCommandBuffers[c]->binCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->uniformsCl, pCommandBuffers[c]->binCl.numBlocks);
			poolFree(&cp->pa, pCommandBuffers[c]);
		}
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
	assert(commandPool);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	_commandPool* cp = (_commandPool*)commandPool;

	//if(cp->usePoolAllocator)
	{
		free(cp->pa.buf);
		free(cp->cpa.buf);
		destroyPoolAllocator(&cp->pa);
		destroyConsecutivePoolAllocator(&cp->cpa);
	}

	free(cp);
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

	sem_destroy((sem_t*)semaphore);
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

	//TODO flush all ops

	_swapchain* s = swapchain;

	for(int c = 0; c < s->numImages; ++c)
	{
		vc4_bo_free(controlFd, s->images[c].handle, 0, s->images->size);
		modeset_destroy_fb(controlFd, &s->images[c]);
	}

	free(s->images);
	free(s);
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
	closeIoctl();
}

