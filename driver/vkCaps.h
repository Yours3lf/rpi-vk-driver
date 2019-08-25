#pragma once

#include <vulkan/vulkan.h>

//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#features-limits
static VkPhysicalDeviceLimits _limits =
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
	.maxVertexInputAttributes = 8, //TODO will fail CTS, MIN is 16
	.maxVertexInputBindings = 8, //TODO will fail CTS, MIN is 16
	.maxVertexInputAttributeOffset = 2047,
	.maxVertexInputBindingStride = 2048,
	.maxVertexOutputComponents = 128, //TODO?
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
	.maxComputeSharedMemorySize = 0, //TODO no compute for now, fails CTS
	.maxComputeWorkGroupCount = {0,0,0}, //TODO no compute for now, fails CTS
	.maxComputeWorkGroupInvocations = 0, //TODO no compute for now, fails CTS
	.maxComputeWorkGroupSize = {0,0,0}, //TODO no compute for now, fails CTS
	.subPixelPrecisionBits = 8,
	.subTexelPrecisionBits = 8,
	.mipmapPrecisionBits = 8,
	.maxDrawIndexedIndexValue = 4294967295,
	.maxDrawIndirectCount = 4294967295,
	.maxSamplerLodBias = 15,
	.maxSamplerAnisotropy = 16.0,
	.maxViewports = 1,
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
	.discreteQueuePriorities = 1, //TODO will fail CTS, MIN is 2
	.pointSizeRange = {1, 64}, //TODO [1] has to be 64 - pointSizeGranularity
	.lineWidthRange = {0.5, 10},
	.pointSizeGranularity = 0.0, //TODO
	.lineWidthGranularity = 0.125,
	.strictLines = 0, //TODO
	.standardSampleLocations = 1,
	.optimalBufferCopyOffsetAlignment = 0x1,
	.optimalBufferCopyRowPitchAlignment = 0x1,
	.nonCoherentAtomSize = 0x40
};

static VkPhysicalDeviceFeatures _features =
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
	.multiViewport = 0,
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

static VkQueueFamilyProperties _queueFamilyProperties[] =
{
	{
		.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT,
		.queueCount = 1,
		.timestampValidBits = 64, //wait timeout is 64bits
		.minImageTransferGranularity = {1, 1, 1}
	}
};
#define numQueueFamilies (sizeof(_queueFamilyProperties)/sizeof(VkQueueFamilyProperties))

static VkSurfaceFormatKHR supportedSurfaceFormats[] =
{
	{
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_PASS_THROUGH_EXT
	},
	{
		.format = VK_FORMAT_R16G16B16A16_SFLOAT,
		.colorSpace = VK_COLOR_SPACE_PASS_THROUGH_EXT
	}
};
#define numSupportedSurfaceFormats (sizeof(supportedSurfaceFormats) / sizeof(VkSurfaceFormatKHR))

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
	//TODO not an official extension, so can't expose it
	//{
	//	.extensionName = "VK_KHR_rpi_surface",
	//	.specVersion = 1
	//}
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
		.extensionName = "VK_KHR_driver_properties",
		.specVersion = 1
	}
};
#define numDeviceExtensions (sizeof(deviceExtensions) / sizeof(VkExtensionProperties))

static VkMemoryType memoryTypes[] =
{
	{
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	},
	{
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		0
	},
};
#define numMemoryTypes (sizeof(memoryTypes) / sizeof(VkMemoryType))

static VkMemoryHeap memoryHeaps[] =
{
	{
		0, //will be filled from /proc/meminfo
		VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
	}
};
#define numMemoryHeaps (sizeof(memoryHeaps) / sizeof(VkMemoryHeap))

static VkFormat unsupportedFormats[] =
{
	VK_FORMAT_BC1_RGB_UNORM_BLOCK, //bc formats
	VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, //etc2 formats
	VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
	VK_FORMAT_EAC_R11_UNORM_BLOCK, //eac formats
	VK_FORMAT_EAC_R11_SNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
	VK_FORMAT_ASTC_4x4_UNORM_BLOCK, //astc formats
	VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
	VK_FORMAT_B8G8R8G8_422_UNORM, //TODO yuv formats???
	VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
	VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
	VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
	VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G16B16G16R16_422_UNORM,
	VK_FORMAT_B16G16R16G16_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
	VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
	VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
	VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
	VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
	VK_FORMAT_R10X6_UNORM_PACK16,
	VK_FORMAT_R12X4_UNORM_PACK16,
	VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, //pvrtc formats
	VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG
};
#define numUnsupportedFormats (sizeof(unsupportedFormats)/sizeof(VkFormat))

#define VK_DRIVER_VERSION VK_MAKE_VERSION(1, 1, 0)
