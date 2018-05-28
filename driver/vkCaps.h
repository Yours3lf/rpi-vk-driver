#pragma once

#include <vulkan/vulkan.h>

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
