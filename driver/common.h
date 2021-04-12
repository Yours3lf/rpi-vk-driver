#pragma once

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <drm/vc4_drm.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include "vkExt.h"

#include "AlignedAllocator.h"
#include "PoolAllocator.h"
#include "ConsecutivePoolAllocator.h"
#include "LinearAllocator.h"
#include "map.h"

#include <stdio.h>
#include "CustomAssert.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "kernelInterface.h"
#include "ControlListUtil.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

#include "vkCaps.h"

#ifndef RPI_PROFILE
	#define RPI_PROFILE 0
#endif

#if RPI_PROFILE == 1
	#define PROFILESTART(x) startMeasure((x), (#x))
	#define PROFILEEND(x) endMeasure((x))
#else
	#define PROFILESTART(x)
	#define PROFILEEND(x)
#endif


/**
//scope
VK_SYSTEM_ALLOCATION_SCOPE_COMMAND
VK_SYSTEM_ALLOCATION_SCOPE_OBJECT
VK_SYSTEM_ALLOCATION_SCOPE_CACHE
VK_SYSTEM_ALLOCATION_SCOPE_DEVICE
VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE
 **/

#define ALLOCATE(size, alignment, scope) (pAllocator == 0) ? calloc(size, 1) : pAllocator->pfnAllocation(pAllocator->pUserData, size, alignment, scope)
#define FREE(memory) (pAllocator == 0) ? free(memory) : pAllocator->pfnFree(pAllocator->pUserData, memory)

#define UNSUPPORTED(str) fprintf(stderr, "Unsupported: %s\n", #str); //exit(-1)
#define UNSUPPORTED_RETURN VK_SUCCESS

typedef struct VkDevice_T _device;

typedef struct VkQueue_T
{
	VK_LOADER_DATA loaderData;
	uint64_t lastEmitSeqno;
	uint64_t lastFinishedSeqno;
	_device* dev;
} _queue;

typedef struct VkCommandPool_T
{
	PoolAllocator pa;
	ConsecutivePoolAllocator cpa;
	uint32_t queueFamilyIndex;
	uint32_t resetAble;
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

typedef struct VkInstance_T _instance;

typedef struct VkPhysicalDevice_T
{
	VK_LOADER_DATA loaderData;
	//apparently unknown physical device extensions can't quite pass anything other than VkPhysicalDevice
	//now that object has to have the loader magic
	//so we just provide a custom data pointer so that our extensions can be used...
	void* customData;
	//hardware id?
	char* path;
	_instance* instance;
} _physicalDevice;

typedef struct VkInstance_T
{
	VK_LOADER_DATA loaderData;
	_physicalDevice dev;
	//supposedly this should contain all the enabled layers?
	int enabledExtensions[numInstanceExtensions];
	int numEnabledExtensions;
	uint32_t technologyVersion;
	uint32_t IDstrUINT;
	uint32_t vpmMemorySize;
	uint32_t hdrSupported;
	uint32_t numSemaphores;
	uint32_t numTMUperSlice;
	uint32_t numQPUperSlice;
	uint32_t numSlices;
	uint32_t v3dRevision;
	uint32_t tileBufferDoubleBufferModeSupported;
	uint32_t tileBufferSize;
	uint32_t vriMemorySize;
	uint32_t hasTiling;
	uint32_t hasControlFlow;
	uint32_t hasEtc1;
	uint32_t hasThreadedFs;
	uint32_t hasMadvise;
	uint32_t hasPerfmon;
	uint32_t hasFixedRCLorder;
} _instance;

typedef struct VkDevice_T
{
	VK_LOADER_DATA loaderData;
	int enabledExtensions[numDeviceExtensions];
	int numEnabledExtensions;
	VkPhysicalDeviceFeatures enabledFeatures;
	_physicalDevice* dev;
	_queue* queues[numQueueFamilies];
	uint32_t numQueues[numQueueFamilies];

	//emulation resources
	VkBuffer emulFsqVertexBuffer;
	VkDeviceMemory emulFsqVertexBufferMemory;
	VkDescriptorPool emulDescriptorPool;
	VkDescriptorSetLayout emulBufferDsl;
	VkDescriptorSetLayout emulTextureDsl;
	VkDescriptorSetLayout emulClearDsl;
	VkSampler emulNearestTextureSampler;
	VkSampler emulLinearTextureSampler;
	VkShaderModule emulBufferToTextureShaderModule;
	VkShaderModule emulTextureToTextureShaderModule;
	//VkShaderModule emulTextureToBufferShaderModule; //TODO
	//VkShaderModule emulBufferToBufferShaderModule; //TODO
	VkShaderModule emulClearShaderModule;
	VkShaderModule emulClearNoColorShaderModule;
} _device;

typedef struct VkRenderPass_T
{
	//collection of:
	//attachments, subpasses, dependencies between subpasses
	//describes how attachments are used in subpasses

	//attachment describes:
	//format, sample count, how contents are treated at start/end of a renderpass

	//subpasses render to same dimensions and fragments

	//framebuffer objects specify views for attachements

	VkAttachmentDescription* attachments;
	uint32_t numAttachments;

	VkSubpassDescription* subpasses;
	uint32_t numSubpasses;

	VkSubpassDependency* subpassDependencies;
	uint32_t numSubpassDependencies;
} _renderpass;

typedef struct VkDeviceMemory_T
{
	uint32_t size;
	uint32_t bo;
	uint32_t memTypeIndex;
	void* mappedPtr;
	uint32_t mappedOffset, mappedSize;
} _deviceMemory;

typedef struct VkBuffer_T
{
	uint32_t size;
	VkBufferUsageFlags usage;
	_deviceMemory* boundMem;
	uint32_t boundOffset;
	uint32_t alignment;
	uint32_t alignedSize;
} _buffer;

typedef struct VkImage_T
{
	VkImageType type; //1d, 2d, 3d
	uint32_t fb; //needed for swapchain
	uint32_t width, height, depth;
	uint32_t miplevels, samples;
	uint32_t levelOffsets[12]; //max 12 mip levels
	uint32_t levelTiling[12];
	uint32_t layers; //number of views for multiview/stereo
	uint32_t size; //overall size including padding and alignment
	uint32_t stride; //the number of bytes from one row of pixels in memory to the next row of pixels in memory (aka pitch)
	uint32_t usageBits;
	uint32_t format;
	uint32_t imageSpace;
	uint32_t tiling; //Linear or T or LT
	uint32_t layout;
	_deviceMemory* boundMem;
	uint32_t boundOffset;
	uint32_t alignment;

	uint32_t concurrentAccess; //TODO
	uint32_t numQueueFamiliesWithAccess;
	uint32_t* queueFamiliesWithAccess;
	uint32_t preTransformMode;
	uint32_t compositeAlpha;
	uint32_t presentMode;
	uint32_t clipped;

	uint32_t flags;
} _image;

typedef struct VkImageView_T
{
	_image* image;
	VkImageViewType viewType;
	VkFormat interpretedFormat;
	VkComponentMapping swizzle;
	VkImageSubresourceRange subresourceRange;
} _imageView;

typedef struct VkSwapchain_T
{
	_image* images;
	uint32_t* inFlight;
	uint32_t numImages;
	uint32_t backbufferIdx;
	VkSurfaceKHR surface;
} _swapchain;

typedef struct VkFramebuffer_T
{
	_renderpass* renderpass;
	_imageView* attachmentViews;
	uint32_t numAttachmentViews;
	uint32_t width, height, layers;
} _framebuffer;

typedef struct VkShaderModule_T
{
	uint32_t bos[VK_RPI_ASSEMBLY_TYPE_MAX];
	uint32_t sizes[VK_RPI_ASSEMBLY_TYPE_MAX];
	//uint64_t* instructions[RPI_ASSEMBLY_TYPE_MAX];
	VkRpiAssemblyMappingEXT* mappings[VK_RPI_ASSEMBLY_TYPE_MAX];
	uint32_t numMappings[VK_RPI_ASSEMBLY_TYPE_MAX];
	uint32_t hasThreadSwitch;
	uint32_t numTextureSamples;
	uint32_t numVaryings;
	uint32_t numFragUniformReads;
	uint32_t numVertUniformReads;
	uint32_t numCoordUniformReads;
	uint32_t numVertVPMreads;
	uint32_t numCoordVPMreads;
	uint32_t numVertVPMwrites;
	uint32_t numCoordVPMwrites;
	uint32_t numFragCycles;
	uint32_t numVertCycles;
	uint32_t numCoordCycles;
	uint32_t numFragALUcycles;
	uint32_t numVertALUcycles;
	uint32_t numCoordALUcycles;
	uint32_t numEmptyFragALUinstructions;
	uint32_t numEmptyVertALUinstructions;
	uint32_t numEmptyCoordALUinstructions;
	uint32_t numFragBranches;
	uint32_t numVertBranches;
	uint32_t numCoordBranches;
	uint32_t numFragSFUoperations;
	uint32_t numVertSFUoperations;
	uint32_t numCoordSFUoperations;
} _shaderModule;

typedef struct VkDescriptorSetLayout_T
{
	//an array of zero or more descriptor bindings
	VkDescriptorSetLayoutBinding* bindings;
	uint32_t bindingsCount;
	VkDescriptorSetLayoutCreateFlags flags;
} _descriptorSetLayout;

typedef struct VkPipelineLayout_T
{
	map descriptorSetBindingMap;
	uint32_t                        setLayoutCount;
	const _descriptorSetLayout*		setLayouts;
	uint32_t                        pushConstantRangeCount;
	const VkPushConstantRange*      pushConstantRanges;
} _pipelineLayout;

typedef struct VkPipeline_T
{
	_shaderModule* modules[6];
	char* names[6];
	uint32_t vertexBindingDescriptionCount;
	VkVertexInputBindingDescription* vertexBindingDescriptions;
	uint32_t vertexAttributeDescriptionCount;
	VkVertexInputAttributeDescription* vertexAttributeDescriptions;
	VkPrimitiveTopology topology;
	VkBool32 primitiveRestartEnable;
	uint32_t viewportCount;
	VkViewport* viewports;
	uint32_t scissorCount;
	VkRect2D* scissors;
	VkBool32 depthClampEnable;
	VkBool32 rasterizerDiscardEnable;
	VkPolygonMode polygonMode;
	VkCullModeFlags cullMode;
	VkFrontFace frontFace;
	VkBool32 depthBiasEnable;
	float depthBiasConstantFactor;
	float depthBiasClamp;
	float depthBiasSlopeFactor;
	float lineWidth;
	VkSampleCountFlagBits rasterizationSamples;
	VkBool32 sampleShadingEnable;
	float minSampleShading;
	VkSampleMask sampleMask;
	VkBool32 alphaToCoverageEnable;
	VkBool32 alphaToOneEnable;
	VkBool32 depthTestEnable;
	VkBool32  depthWriteEnable;
	VkCompareOp depthCompareOp;
	VkBool32 depthBoundsTestEnable;
	VkBool32 stencilTestEnable;
	VkStencilOpState front;
	VkStencilOpState back;
	float minDepthBounds;
	float maxDepthBounds;
	VkBool32 logicOpEnable;
	VkLogicOp logicOp;
	uint32_t attachmentCount;
	VkPipelineColorBlendAttachmentState* attachmentBlendStates;
	float blendConstants[4];
	uint32_t dynamicStateCount;
	VkDynamicState* dynamicStates;
	_pipelineLayout* layout;
	_renderpass* renderPass;
	uint32_t subpass;
} _pipeline;

typedef struct VkCommandBuffer_T
{
	VK_LOADER_DATA loaderData;

	_device* dev; //device from which it was created

	VkCommandBufferLevel level;

	//Recorded commands include commands to bind pipelines and descriptor sets to the command buffer, commands to modify dynamic state, commands to draw (for graphics rendering),
	//commands to dispatch (for compute), commands to execute secondary command buffers (for primary command buffers only), commands to copy buffers and images, and other commands

	//Rpi only supports vertex and pixel shaders
	//(coordinate shaders will just use the vertex shader push constants)
	//anything else will be ignored I guess
	char pushConstantBufferVertex[256];
	char pushConstantBufferPixel[256];

	ControlList binCl;
	ControlList shaderRecCl;
	uint32_t shaderRecCount;
	ControlList uniformsCl;
	ControlList handlesCl;
	commandBufferState state;
	VkCommandBufferUsageFlags usageFlags;
	_commandPool* cp;

	ControlList uniformRelocCl;
	ControlList gemRelocCl;
	ControlList shaderRecRelocCl;

	//State data
	_pipeline* graphicsPipeline;
	_pipeline* computePipeline;

	_renderpass* currRenderPass;

	VkViewport viewport;
	VkRect2D scissor;
	float lineWidth;
	float depthBiasConstantFactor;
	float depthBiasClamp;
	float depthBiasSlopeFactor;
	float blendConstants[4];
	float minDepthBounds;
	float maxDepthBounds;
	uint32_t stencilCompareMask[2];
	uint32_t stencilWriteMask[2];
	uint32_t stencilReference[2];

	uint32_t vertexBufferOffsets[8];
	_buffer* vertexBuffers[8];

	uint32_t indexBufferOffset;
	_buffer* indexBuffer;

	//Renderpass scope query must begin outside renderpass
	//so there won't be any current marker...
	//therefore store perfmonID here, and copy on beginrenderpass
	//into marker
	void* perfmonID;

	//dirty flags used to reduce command stream clutter
	uint32_t vertexBufferDirty;
	uint32_t indexBufferDirty;
	uint32_t viewportDirty;
	uint32_t lineWidthDirty;
	uint32_t depthBiasDirty;
	uint32_t graphicsPipelineDirty;
	uint32_t computePipelineDirty;
	uint32_t subpassDirty;
	uint32_t blendConstantsDirty;
	uint32_t scissorDirty;
	uint32_t depthBoundsDirty;
	uint32_t stencilCompareMaskDirty;
	uint32_t stencilWriteMaskDirty;
	uint32_t stencilReferenceDirty;
	uint32_t descriptorSetDirty;
	uint32_t pushConstantDirty;
} _commandBuffer;

typedef struct VkFence_T
{
	uint64_t seqno;
	uint32_t signaled;
} _fence;

typedef struct VkBufferView_T
{
	_buffer*                   buffer;
	VkFormat                   format;
	VkDeviceSize               offset;
	VkDeviceSize               range;
} _bufferView;

typedef struct VkSampler_T
{
	VkFilter minFilter, magFilter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode addressModeU, addressModeV, addressModeW;
	float mipLodBias;
	uint32_t disableAutoLod;
	VkBool32 anisotropyEnable;
	float maxAnisotropy;
	VkBool32 compareEnable;
	VkCompareOp compareOp;
	float minLod, maxLod;
	VkBorderColor borderColor;
	VkBool32 unnormalizedCoordinates;
} _sampler;

typedef struct VkDescriptorImage_T
{
	uint32_t count;
	VkDescriptorType type;
	VkShaderStageFlags stageFlags;
	_sampler* sampler;
	_imageView* imageView;
	VkImageLayout imageLayout;
} _descriptorImage;

typedef struct VkDescriptorBuffer_T
{
	uint32_t count;
	VkDescriptorType type;
	VkShaderStageFlags stageFlags;
	_buffer* buffer;
	VkDeviceSize offset;
	VkDeviceSize range;
} _descriptorBuffer;

typedef struct VkDescriptorTexelBuffer_T
{
	uint32_t count;
	VkDescriptorType type;
	VkShaderStageFlags stageFlags;
	_bufferView* bufferView;
} _descriptorTexelBuffer;

typedef struct VkDescriptorSet_T
{
	//VkDescriptorSetLayoutCreateFlags flags;

	map imageBindingMap;
	map bufferBindingMap;
	map texelBufferBindingMap;

	//pointers into CPAs

	//VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	//VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
	//VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
	_descriptorImage* imageDescriptors;

	//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
	//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
	_descriptorBuffer* bufferDescriptors;

	//VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
	_descriptorTexelBuffer* texelBufferDescriptors;

	uint32_t imageDescriptorsCount;
	uint32_t bufferDescriptorsCount;
	uint32_t texelBufferDescriptorsCount;
} _descriptorSet;

typedef struct VkDescriptorPool_T
{
	PoolAllocator descriptorSetPA;
	ConsecutivePoolAllocator mapElementCPA;
	ConsecutivePoolAllocator imageDescriptorCPA;
	ConsecutivePoolAllocator bufferDescriptorCPA;
	ConsecutivePoolAllocator texelBufferDescriptorCPA;

	uint32_t freeAble;
} _descriptorPool;

typedef struct VkQuery_T
{
	uint32_t enabledCounters[VC4_PERFCNT_NUM_EVENTS];
	uint64_t counterValues[2][DRM_VC4_MAX_PERF_COUNTERS];
	uint32_t numEnabledCounters;
	uint32_t perfmonIDs[2];
} _query;

typedef struct VkQueryPool_T
{
	VkQueryType type;
	uint32_t queryCount;
	_query* queryPool;
} _queryPool;

typedef struct VkDisplayModeKHR_T
{
	uint32_t connectorID;
	uint32_t modeID;
} _displayMode;

uint32_t getFormatBpp(VkFormat f);
uint32_t packVec4IntoABGR8(const float rgba[4]);
void createImageBO(_image* i);
int findInstanceExtension(const char* name);
int findDeviceExtension(const char* name);
uint32_t isLTformat(uint32_t bpp, uint32_t width, uint32_t height);
void getUTileDimensions(uint32_t bpp, uint32_t* tileW, uint32_t* tileH);
uint32_t roundUp(uint32_t numToRound, uint32_t multiple);
int isDepthStencilFormat(VkFormat format);
uint32_t getCompareOp(VkCompareOp op);
uint32_t getStencilOp(VkStencilOp op);
uint32_t getTopology(VkPrimitiveTopology topology);
uint32_t getPrimitiveMode(VkPrimitiveTopology topology);
uint32_t ulog2(uint32_t v);
void encodeTextureUniform(uint32_t* params,
						  uint8_t numMipLevels,
						  uint8_t textureDataType,
						  uint8_t isCubeMap,
						  uint32_t cubemapStride,
						  uint32_t textureBasePtr,
						  uint16_t height,
						  uint16_t width,
						  uint8_t minFilter,
						  uint8_t magFilter,
						  uint8_t wrapT,
						  uint8_t wrapS,
						  uint8_t noAutoLod);
void encodeStencilValue(uint32_t* values, uint32_t* numValues, VkStencilOpState front, VkStencilOpState back, uint8_t stencilTestEnable);
uint32_t encodeVPMSetup(uint8_t stride,
						uint8_t direction,
						uint8_t isLaned,
						uint8_t size,
						uint8_t address,
						uint8_t vectorComponentsToRead);
uint8_t getTextureDataType(VkFormat format);
uint8_t getMinFilterType(VkFilter minFilter, VkSamplerMipmapMode mipFilter);
uint8_t getWrapMode(VkSamplerAddressMode mode);
uint32_t getRenderTargetFormatVC4(VkFormat format);
void clFit(ControlList* cl, uint32_t commandSize);
void clDump(void* cl, uint32_t size);
void setupEmulationResources(VkDevice device);
void setupClearEmulationResources(VkDevice device);
uint32_t getPow2Pad(uint32_t n);
