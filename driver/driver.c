#include <stdio.h>
#include "CustomAssert.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <vulkan/vulkan.h>
#include "vkExt.h"

#include "modeset.h"
#include "kernelInterface.h"

#include "AlignedAllocator.h"
#include "PoolAllocator.h"
#include "LinearAllocator.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

VkPhysicalDeviceLimits _limits =
{
	//TODO these values might change
	.maxImageDimension1D = 16384,
	.maxImageDimension2D = 16384,
	.maxImageDimension3D = 2084,
	.maxImageDimensionCube = 16384,
	.maxImageArrayLayers = 2048,
	.maxTexelBufferElements = 134217728,
	.maxUniformBufferRange = 65536,
	.maxStorageBufferRange = 4294967295,
	.maxPushConstantsSize = 256,
	.maxMemoryAllocationCount = 4096,
	.maxSamplerAllocationCount = 4000,
	.bufferImageGranularity = 0x400, //TODO 1KB?
	.sparseAddressSpaceSize = 0xffffffff, //32 bits
	.maxBoundDescriptorSets = 8,
	.maxPerStageDescriptorSamplers = 4000,
	.maxPerStageDescriptorUniformBuffers = 12,
	.maxPerStageDescriptorStorageBuffers = 4096,
	.maxPerStageDescriptorSampledImages = 16384,
	.maxPerStageDescriptorStorageImages = 16384,
	.maxPerStageDescriptorInputAttachments = 8, //TODO
	.maxPerStageResources = 53268,
	.maxDescriptorSetSamplers = 4000,
	.maxDescriptorSetUniformBuffers = 72, //TODO
	.maxDescriptorSetUniformBuffersDynamic = 72,
	.maxDescriptorSetStorageBuffers = 4096,
	.maxDescriptorSetStorageBuffersDynamic = 16,
	.maxDescriptorSetSampledImages = 98304,
	.maxDescriptorSetStorageImages = 98304,
	.maxDescriptorSetInputAttachments = 8, //TODO
	.maxVertexInputAttributes = 32,
	.maxVertexInputBindings = 32,
	.maxVertexInputAttributeOffset = 2047,
	.maxVertexInputBindingStride = 2048,
	.maxVertexOutputComponents = 128,
	.maxTessellationGenerationLevel = 0, //No tessellation
	.maxTessellationPatchSize = 0,
	.maxTessellationControlPerVertexInputComponents = 0,
	.maxTessellationControlPerVertexOutputComponents = 0,
	.maxTessellationControlPerPatchOutputComponents = 0,
	.maxTessellationControlTotalOutputComponents = 0,
	.maxTessellationEvaluationInputComponents = 0,
	.maxTessellationEvaluationOutputComponents = 0,
	.maxGeometryShaderInvocations = 0, //TODO no geometry shaders for now
	.maxGeometryInputComponents = 0,
	.maxGeometryOutputComponents = 0,
	.maxGeometryOutputVertices = 0,
	.maxGeometryTotalOutputComponents = 0,
	.maxFragmentInputComponents = 128,
	.maxFragmentOutputAttachments = 8,
	.maxFragmentDualSrcAttachments = 1,
	.maxFragmentCombinedOutputResources = 16,
	.maxComputeSharedMemorySize = 0, //TODO no compute for now
	.maxComputeWorkGroupCount = {0,0,0},
	.maxComputeWorkGroupInvocations = 0,
	.maxComputeWorkGroupSize = {0,0,0},
	.subPixelPrecisionBits = 8,
	.subTexelPrecisionBits = 8,
	.mipmapPrecisionBits = 8,
	.maxDrawIndexedIndexValue = 4294967295,
	.maxDrawIndirectCount = 4294967295,
	.maxSamplerLodBias = 15,
	.maxSamplerAnisotropy = 16.0,
	.maxViewports = 16,
	.maxViewportDimensions = {16384,16384},
	.viewportBoundsRange = {-32768,32768},
	.viewportSubPixelBits = 8,
	.minMemoryMapAlignment = 0x40, //TODO
	.minTexelBufferOffsetAlignment = 0x10,
	.minUniformBufferOffsetAlignment = 0x100,
	.minStorageBufferOffsetAlignment = 0x20,
	.minTexelOffset = -8,
	.maxTexelOffset = 7,
	.minTexelGatherOffset = -32,
	.maxTexelGatherOffset = 31,
	.minInterpolationOffset = -0.5,
	.maxInterpolationOffset = 0.4375,
	.subPixelInterpolationOffsetBits = 4,
	.maxFramebufferWidth = 16384,
	.maxFramebufferHeight = 16384,
	.maxFramebufferLayers = 2048,
	.framebufferColorSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.framebufferDepthSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.framebufferStencilSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.framebufferNoAttachmentsSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.maxColorAttachments = 8,
	.sampledImageColorSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.sampledImageIntegerSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.sampledImageDepthSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.sampledImageStencilSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.storageImageSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT,
	.maxSampleMaskWords = 1,
	.timestampComputeAndGraphics = 1,
	.timestampPeriod = 1,
	.maxClipDistances = 8,
	.maxCullDistances = 8,
	.maxCombinedClipAndCullDistances = 8,
	.discreteQueuePriorities = 1,
	.pointSizeRange = {1, 189.875},
	.lineWidthRange = {0.5, 10},
	.pointSizeGranularity = 0.125,
	.lineWidthGranularity = 0.125,
	.strictLines = 0, //TODO
	.standardSampleLocations = 1,
	.optimalBufferCopyOffsetAlignment = 0x1,
	.optimalBufferCopyRowPitchAlignment = 0x1,
	.nonCoherentAtomSize = 0x40
};

VkPhysicalDeviceFeatures _features =
{
	//TODO this might change
	.robustBufferAccess = 1,
	.fullDrawIndexUint32 = 1, //TODO
	.imageCubeArray = 1, //TODO
	.independentBlend = 1,
	.geometryShader = 0,
	.tessellationShader = 0,
	.sampleRateShading = 1, //TODO
	.dualSrcBlend = 1,
	.logicOp = 1,
	.multiDrawIndirect = 1,
	.drawIndirectFirstInstance = 1,
	.depthClamp = 1,
	.depthBiasClamp = 1,
	.fillModeNonSolid = 1,
	.depthBounds = 1,
	.wideLines = 1,
	.largePoints = 1,
	.alphaToOne = 1,
	.multiViewport = 1,
	.samplerAnisotropy = 1,
	.textureCompressionETC2 = 0,
	.textureCompressionASTC_LDR = 0,
	.textureCompressionBC = 0,
	.occlusionQueryPrecise = 1,
	.pipelineStatisticsQuery = 1,
	.vertexPipelineStoresAndAtomics = 1,
	.fragmentStoresAndAtomics = 1,
	.shaderTessellationAndGeometryPointSize = 0,
	.shaderImageGatherExtended = 1,
	.shaderStorageImageExtendedFormats = 1,
	.shaderStorageImageMultisample = 1,
	.shaderStorageImageReadWithoutFormat = 0,
	.shaderStorageImageWriteWithoutFormat = 0,
	.shaderUniformBufferArrayDynamicIndexing = 1,
	.shaderSampledImageArrayDynamicIndexing = 1,
	.shaderStorageBufferArrayDynamicIndexing = 1,
	.shaderStorageImageArrayDynamicIndexing = 1,
	.shaderClipDistance = 1,
	.shaderCullDistance = 1,
	.shaderFloat64 = 0,
	.shaderInt64 = 0,
	.shaderInt16 = 0,
	.shaderResourceResidency = 1,
	.shaderResourceMinLod = 1,
	.sparseBinding = 1,
	.sparseResidencyBuffer = 1,
	.sparseResidencyImage2D = 1,
	.sparseResidencyImage3D = 1,
	.sparseResidency2Samples = 1,
	.sparseResidency4Samples = 1,
	.sparseResidency8Samples = 1,
	.sparseResidency16Samples = 0,
	.sparseResidencyAliased = 1,
	.variableMultisampleRate = 1,
	.inheritedQueries = 1,
};
#define numFeatures (sizeof(_features)/sizeof(VkBool32))

typedef struct VkPhysicalDevice_T
{
	//hardware id?
	int dummy;
} _physicalDevice;

typedef struct VkQueue_T
{
	int dummy;
} _queue;

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

	struct drm_vc4_submit_cl cls[100]; //each cl is a draw call
	unsigned numClsUsed;
	commandBufferState state;
	VkCommandBufferUsageFlags usageFlags;
} _commandBuffer;

typedef struct VkCommandPool_T
{
	int usePoolAllocator;
	PoolAllocator pa;
	LinearAllocator la;
} _commandPool;

VkQueueFamilyProperties _queueFamilyProperties[] =
{
	{
		.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT,
		.queueCount = 1,
		.timestampValidBits = 64, //TODO
		.minImageTransferGranularity = {1, 1, 1}
	}
};
#define numQueueFamilies (sizeof(_queueFamilyProperties)/sizeof(VkQueueFamilyProperties))

static VkExtensionProperties instanceExtensions[] =
{
	{
		.extensionName = "VK_KHR_surface",
		.specVersion = 25
	},
	{
		.extensionName = "VK_KHR_display",
		.specVersion = 21
	},
	{
		.extensionName = "VK_EXT_direct_mode_display",
		.specVersion = 1
	},
	{
		.extensionName = "VK_EXT_debug_report",
		.specVersion = 9
	},
	{
		.extensionName = "VK_EXT_debug_utils",
		.specVersion = 1
	},
	{
		.extensionName = "VK_KHR_rpi_surface",
		.specVersion = 1
	}
};
#define numInstanceExtensions (sizeof(instanceExtensions) / sizeof(VkExtensionProperties))

static VkExtensionProperties deviceExtensions[] =
{
	{
		.extensionName = "VK_KHR_display_swapchain",
		.specVersion = 9
	},
	{
		.extensionName = "VK_KHR_maintenance1",
		.specVersion = 2
	},
	{
		.extensionName = "VK_KHR_maintenance2",
		.specVersion = 1
	},
	{
		.extensionName = "VK_KHR_maintenance3",
		.specVersion = 1
	},
	{
		.extensionName = "VK_KHR_swapchain",
		.specVersion = 70
	},
	{
		.extensionName = "VK_EXT_debug_marker",
		.specVersion = 4
	},
	{
		.extensionName = "VK_EXT_display_control",
		.specVersion = 1
	}
};
#define numDeviceExtensions (sizeof(deviceExtensions) / sizeof(VkExtensionProperties))

typedef struct VkInstance_T
{
	//supposedly this should contain all the enabled layers?
	int enabledExtensions[numInstanceExtensions];
	int numEnabledExtensions;
	_physicalDevice dev;
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

int findInstanceExtension(char* name)
{
	for(int c = 0; c < numInstanceExtensions; ++c)
	{
		if(strcmp(instanceExtensions[c].extensionName, name) == 0)
		{
			return c;
		}
	}

	return -1;
}

int findDeviceExtension(char* name)
{
	for(int c = 0; c < numDeviceExtensions; ++c)
	{
		if(strcmp(deviceExtensions[c].extensionName, name) == 0)
		{
			return c;
		}
	}

	return -1;
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

	int chip_info = vc4_get_chip_info(renderFd);
	int has_tiling = vc4_test_tiling(renderFd);

	int has_control_flow = vc4_has_feature(renderFd, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
	int has_etc1 = vc4_has_feature(renderFd, DRM_VC4_PARAM_SUPPORTS_ETC1);
	int has_threaded_fs = vc4_has_feature(renderFd, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
	int has_madvise = vc4_has_feature(renderFd, DRM_VC4_PARAM_SUPPORTS_MADVISE);

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
	assert(pSwapchain);

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
	assert(pSwapchainImageCount);

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

	//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	//specifies that command buffers allocated from the pool will be short-lived, meaning that they will be reset or freed in a relatively short timeframe.
	//This flag may be used by the implementation to control memory allocation behavior within the pool.
	//--> definitely use pool allocator

	//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	//allows any command buffer allocated from a pool to be individually reset to the initial state; either by calling vkResetCommandBuffer, or via the implicit reset when calling vkBeginCommandBuffer.
	//If this flag is not set on a pool, then vkResetCommandBuffer must not be called for any command buffer allocated from that pool.

	//TODO pool family ignored for now

	_commandPool* cp = malloc(sizeof(_commandPool));

	//initial number of command buffers to hold
	int numCommandBufs = 100;

	if(pCreateInfo->flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	{
		//use pool allocator
		cp->usePoolAllocator = 1;
		cp->pa = createPoolAllocator(malloc(numCommandBufs * sizeof(_commandBuffer)), sizeof(_commandBuffer), numCommandBufs * sizeof(_commandBuffer));
	}
	else
	{
		cp->usePoolAllocator = 0;
		cp->la = createLinearAllocator(malloc(numCommandBufs * sizeof(_commandBuffer)), numCommandBufs * sizeof(_commandBuffer));
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

	if(cp->usePoolAllocator)
	{
		for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
		{
			pCommandBuffers[c] = poolAllocte(&cp->pa);
			pCommandBuffers[c]->numClsUsed = 0;
			pCommandBuffers[c]->usageFlags = 0;
			pCommandBuffers[c]->state = CMDBUF_STATE_INITIAL;

			if(!pCommandBuffers[c])
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY; //TODO or VK_ERROR_OUT_OF_DEVICE_MEMORY?
				break;
			}
		}
	}
	else
	{
		for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
		{
			pCommandBuffers[c] = linearAllocte(&cp->la, sizeof(_commandBuffer));
			pCommandBuffers[c]->numClsUsed = 0;
			pCommandBuffers[c]->usageFlags = 0;
			pCommandBuffers[c]->state = CMDBUF_STATE_INITIAL;

			if(!pCommandBuffers[c])
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY; //TODO or VK_ERROR_OUT_OF_DEVICE_MEMORY?
				break;
			}
		}
	}

	if(res != VK_SUCCESS)
	{
		if(cp->usePoolAllocator)
		{
			for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
			{
				poolFree(&cp->pa, pCommandBuffers[c]);
				pCommandBuffers[c] = 0;
			}
		}
		else
		{
			for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
			{
				//we don't really free linear memory, just reset the whole command pool
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

	//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	//specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission.

	//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
	//specifies that a secondary command buffer is considered to be entirely inside a render pass. If this is a primary command buffer, then this bit is ignored

	//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
	//specifies that a command buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary command buffers

	//When a command buffer begins recording, all state in that command buffer is undefined

	commandBuffer->usageFlags = pBeginInfo->flags;
	commandBuffer->numClsUsed = 0;
	commandBuffer->state = CMDBUF_STATE_RECORDING;

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

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_PENDING;
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

	for(int c = 0; c < pPresentInfo->swapchainCount; ++c)
	{
		//TODO
		modeset_swapbuffer(controlFd, (modeset_dev*)pPresentInfo->pSwapchains[c], pPresentInfo->pImageIndices[c]);
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

	_commandPool* cp = (_commandPool*)commandPool;

	for(int c = 0; c < commandBufferCount; ++c)
	{
		if(cp->usePoolAllocator)
		{
			poolFree(&cp->pa, pCommandBuffers[c]);
		}
		else
		{
			linearFree(&cp->la, pCommandBuffers[c]);
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

	if(cp->usePoolAllocator)
	{
		free(cp->pa.buf);
		destroyPoolAllocator(&cp->pa);
	}
	else
	{
		free(cp->la.buf);
		destroyLinearAllocator(&cp->la);
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
	closeIoctl();
}

