#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#features-limits
static VkPhysicalDeviceLimits _limits =
{
	//TODO we could use these hard limits to optimise some of the driver code
	//eg. staticcally allocated buffers instead of dynamically growing ones
	.maxImageDimension1D = 2048,
	.maxImageDimension2D = 2048,
	.maxImageDimension3D = 2084,
	.maxImageDimensionCube = 2048,
	.maxImageArrayLayers = 2048,
	.maxTexelBufferElements = 134217728,
	.maxUniformBufferRange = 4294967295, //handled as buffers
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
	.maxPerStageDescriptorInputAttachments = 8,
	.maxPerStageResources = 53268,
	.maxDescriptorSetSamplers = 4000,
	.maxDescriptorSetUniformBuffers = 72,
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
	.maxVertexOutputComponents = 128,
	.maxTessellationGenerationLevel = 0, //No tessellation
	.maxTessellationPatchSize = 0,
	.maxTessellationControlPerVertexInputComponents = 0,
	.maxTessellationControlPerVertexOutputComponents = 0,
	.maxTessellationControlPerPatchOutputComponents = 0,
	.maxTessellationControlTotalOutputComponents = 0,
	.maxTessellationEvaluationInputComponents = 0,
	.maxTessellationEvaluationOutputComponents = 0,
	.maxGeometryShaderInvocations = 0, //no geometry shaders
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
	.maxDrawIndexedIndexValue = 65535,
	.maxDrawIndirectCount = 0,
	.maxSamplerLodBias = 15,
	.maxSamplerAnisotropy = 1.0,
	.maxViewports = 1,
	.maxViewportDimensions = {4096,4096},
	.viewportBoundsRange = {-8192,8192},
	.viewportSubPixelBits = 8,
	.minMemoryMapAlignment = 0x40, //TODO
	.minTexelBufferOffsetAlignment = 0x10,
	.minUniformBufferOffsetAlignment = 0x100,
	.minStorageBufferOffsetAlignment = 0x20,
	.minTexelOffset = -8,
	.maxTexelOffset = 7,
	.minTexelGatherOffset = 0,
	.maxTexelGatherOffset = 0,
	.minInterpolationOffset = -0.5,
	.maxInterpolationOffset = 0.4375,
	.subPixelInterpolationOffsetBits = 4,
	.maxFramebufferWidth = 4096,
	.maxFramebufferHeight = 4096,
	.maxFramebufferLayers = 2048,
	.framebufferColorSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.framebufferDepthSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.framebufferStencilSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.framebufferNoAttachmentsSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.maxColorAttachments = 8,
	.sampledImageColorSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.sampledImageIntegerSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.sampledImageDepthSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.sampledImageStencilSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
	.storageImageSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT,
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
	.robustBufferAccess = 0,
	.fullDrawIndexUint32 = 0,
	.imageCubeArray = 1, //TODO
	.independentBlend = 1,
	.geometryShader = 0,
	.tessellationShader = 0,
	.sampleRateShading = 0,
	.dualSrcBlend = 1,
	.logicOp = 0, //TODO
	.multiDrawIndirect = 0,
	.drawIndirectFirstInstance = 0,
	.depthClamp = 1,
	.depthBiasClamp = 1,
	.fillModeNonSolid = 1,
	.depthBounds = 1,
	.wideLines = 1,
	.largePoints = 1,
	.alphaToOne = 1,
	.multiViewport = 0,
	.samplerAnisotropy = 0,
	.textureCompressionETC2 = 1,
	.textureCompressionASTC_LDR = 0,
	.textureCompressionBC = 0,
	.occlusionQueryPrecise = 0,
	.pipelineStatisticsQuery = 1,
	.vertexPipelineStoresAndAtomics = 0,
	.fragmentStoresAndAtomics = 0,
	.shaderTessellationAndGeometryPointSize = 0,
	.shaderImageGatherExtended = 0,
	.shaderStorageImageExtendedFormats = 0,
	.shaderStorageImageMultisample = 0,
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
	.shaderResourceResidency = 0,
	.shaderResourceMinLod = 0,
	.sparseBinding = 0,
	.sparseResidencyBuffer = 0,
	.sparseResidencyImage2D = 0,
	.sparseResidencyImage3D = 0,
	.sparseResidency2Samples = 0,
	.sparseResidency4Samples = 0,
	.sparseResidency8Samples = 0,
	.sparseResidency16Samples = 0,
	.sparseResidencyAliased = 0,
	.variableMultisampleRate = 0,
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
	},
	{
		.format = VK_FORMAT_B5G6R5_UNORM_PACK16,
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
	},
	{
		.extensionName = "VK_KHR_performance_query",
		.specVersion = 1
	}
};
#define numDeviceExtensions (sizeof(deviceExtensions) / sizeof(VkExtensionProperties))

static VkMemoryType memoryTypes[] =
{
	//TODO might change this
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

static VkFormat supportedFormats[] =
{
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R8G8B8_UNORM,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	VK_FORMAT_R5G6B5_UNORM_PACK16,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R16_SFLOAT,
	VK_FORMAT_R16_SINT,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_SINT,
	VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
	VK_FORMAT_G8B8G8R8_422_UNORM
};
#define numSupportedFormats (sizeof(supportedFormats)/sizeof(VkFormat))

static VkPerformanceCounterKHR performanceCounterTypes[] =
{ //TODO UUID
	{
		.unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
		.scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
		.storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	},
	{
	  .unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	  .scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	  .storage = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	}
};

static VkPerformanceCounterDescriptionKHR performanceCounterDescriptions[] =
{
	{
		.name = "FRONT_END_PIPELINE_VALID_PRIMS_NO_RENDER",
	},
	{
		.name = "FRONT_END_PIPELINE_VALID_PRIMS_RENDER",
	},
	{
		.name = "FRONT_END_PIPELINE_CLIPPED_QUADS",
	},
	{
		.name = "FRONT_END_PIPELINE_VALID_QUADS",
	},
	{
		.name = "TILE_BUFFER_QUADS_NOT_PASSING_STENCIL",
	},
	{
		.name = "TILE_BUFFER_QUADS_NOT_PASSING_Z_AND_STENCIL",
	},
	{
		.name = "TILE_BUFFER_QUADS_PASSING_Z_AND_STENCIL",
	},
	{
		.name = "TILE_BUFFER_QUADS_ZERO_COVERAGE",
	},
	{
		.name = "TILE_BUFFER_QUADS_NON_ZERO_COVERAGE",
	},
	{
		.name = "TILE_BUFFER_QUADS_WRITTEN_TO_COLOR_BUF",
	},
	{
		.name = "PLB_PRIMS_OUTSIDE_VIEWPORT",
	},
	{
		.name = "PLB_PRIMS_NEED_CLIPPING",
	},
	{
		.name = "PRIMITIVE_SETUP_ENGINE_PRIMS_REVERSED",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_IDLE_CYCLES",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_VERTEX_COORD_SHADING",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_FRAGMENT_SHADING",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_EXEC_VALID_INST",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_WAITING_TMUS",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_WAITING_SCOREBOARD",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_CLK_CYCLES_WAITING_VARYINGS",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_INST_CACHE_HIT",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_INST_CACHE_MISS",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_UNIFORM_CACHE_HIT",
	},
	{
		.name = "QUAD_PROCESSOR_UNIT_TOTAL_UNIFORM_CACHE_MISS",
	},
	{
		.name = "TEXTURE_MEMORY_LOOKUP_UNIT_TOTAL_TEXT_QUADS_PROCESSED",
	},
	{
		.name = "TEXTURE_MEMORY_LOOKUP_UNIT_TOTAL_TEXT_CACHE_MISS",
	},
	{
		.name = "VERTEX_PIPE_MEMORY_TOTAL_CLK_CYCLES_VERTEX_DMA_WRITE_STALLED",
	},
	{
		.name = "VERTEX_PIPE_MEMORY_TOTAL_CLK_CYCLES_VERTEX_DMA_STALLED",
	},
	{
		.name = "L2C_TOTAL_L2_CACHE_HIT",
	},
	{
		.name = "L2C_TOTAL_L2_CACHE_MISS",
	}
};

#define numPerformanceCounterTypes (sizeof(performanceCounterTypes)/sizeof(uint32_t))

#define VK_DRIVER_VERSION VK_MAKE_VERSION(1, 1, 0)
