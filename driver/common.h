#pragma once

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <drm/vc4_drm.h>

#include <vulkan/vulkan.h>
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

/**
//scope
VK_SYSTEM_ALLOCATION_SCOPE_COMMAND
VK_SYSTEM_ALLOCATION_SCOPE_OBJECT
VK_SYSTEM_ALLOCATION_SCOPE_CACHE
VK_SYSTEM_ALLOCATION_SCOPE_DEVICE
VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE
 **/

#define ALLOCATE(size, alignment, scope) (pAllocator == 0) ? malloc(size) : pAllocator->pfnAllocation(pAllocator->pUserData, size, alignment, scope)
#define FREE(memory) (pAllocator == 0) ? free(memory) : pAllocator->pfnFree(pAllocator->pUserData, memory)

#define UNSUPPORTED(str) fprintf(stderr, "Unsupported: %s\n", str); exit(-1)

typedef struct VkDevice_T _device;

typedef struct VkQueue_T
{
	uint64_t lastEmitSeqno;
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
	//hardware id?
	char* path;
	_instance* instance;
} _physicalDevice;

typedef struct VkInstance_T
{
	_physicalDevice dev;
	//supposedly this should contain all the enabled layers?
	int enabledExtensions[numInstanceExtensions];
	int numEnabledExtensions;
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
	uint32_t paddedWidth, paddedHeight;
	uint32_t miplevels, samples;
	uint32_t layers; //number of views for multiview/stereo
	uint32_t size; //overall size including padding and alignment
	uint32_t stride; //the number of bytes from one row of pixels in memory to the next row of pixels in memory (aka pitch)
	uint32_t usageBits;
	uint32_t format;
	uint32_t imageSpace;
	uint32_t tiling; //T or LT
	uint32_t needToClear;
	uint32_t clearColor[2];
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

typedef enum RpiAssemblyType {
	RPI_ASSEMBLY_TYPE_COORDINATE = 0,
	RPI_ASSEMBLY_TYPE_VERTEX = 1,
	RPI_ASSEMBLY_TYPE_FRAGMENT = 2,
	RPI_ASSEMBLY_TYPE_COMPUTE = 3,
	RPI_ASSEMBLY_TYPE_MAX,
} RpiAssemblyType;

typedef struct VkShaderModule_T
{
	uint32_t bos[RPI_ASSEMBLY_TYPE_MAX];
	uint32_t sizes[RPI_ASSEMBLY_TYPE_MAX];
	VkRpiAssemblyMappingEXT* mappings;
	uint32_t numMappings;
	uint32_t hasThreadSwitch;
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
	_device* dev; //device from which it was created

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

	VkRect2D renderArea;
	_renderpass* renderpass;
	_framebuffer* fbo;
	uint32_t currentSubpass;
	_pipeline* graphicsPipeline;
	_pipeline* computePipeline;

	uint32_t numDrawCallsSubmitted;

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
	ConsecutivePoolAllocator* imageDescriptorCPA;
	ConsecutivePoolAllocator* bufferDescriptorCPA;
	ConsecutivePoolAllocator* texelBufferDescriptorCPA;

	uint32_t freeAble;
} _descriptorPool;

uint32_t getFormatBpp(VkFormat f);
uint32_t packVec4IntoABGR8(const float rgba[4]);
void createImageBO(_image* i);
int findInstanceExtension(char* name);
int findDeviceExtension(char* name);
void getPaddedTextureDimensionsT(uint32_t width, uint32_t height, uint32_t bpp, uint32_t* paddedWidth, uint32_t* paddedHeight);
int isDepthStencilFormat(VkFormat format);
uint32_t getDepthCompareOp(VkCompareOp op);
uint32_t getTopology(VkPrimitiveTopology topology);
uint32_t getPrimitiveMode(VkPrimitiveTopology topology);
uint32_t getFormatByteSize(VkFormat format);
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
uint8_t getTextureDataType(VkFormat format);
uint8_t getMinFilterType(VkFilter minFilter, VkSamplerMipmapMode mipFilter, float maxLod);
uint8_t getWrapMode(VkSamplerAddressMode mode);
uint32_t getRenderTargetFormatVC4(VkFormat format);
void clFit(VkCommandBuffer cb, ControlList* cl, uint32_t commandSize);
void clDump(void* cl, uint32_t size);
