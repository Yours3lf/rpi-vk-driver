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
	uint32_t handle;
	uint32_t fb; //needed for swapchain
	uint32_t width, height, depth;
	uint32_t paddedWidth, paddedHeight;
	uint32_t miplevels, samples;
	uint32_t layers; //number of views for multiview/stereo
	uint32_t size; //overall size including padding
	uint32_t stride; //the number of bytes from one row of pixels in memory to the next row of pixels in memory (aka pitch)
	uint32_t usageBits;
	uint32_t format;
	uint32_t imageSpace;
	uint32_t tiling;
	uint32_t needToClear;
	uint32_t clearColor[2];
	uint32_t layout;
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

typedef struct VkShaderModule_T
{
	uint32_t bos[VK_RPI_ASSEMBLY_TYPE_MAX];
	uint32_t sizes[VK_RPI_ASSEMBLY_TYPE_MAX];
} _shaderModule;

typedef struct VkPipeline_T
{
	VkShaderModule modules[6];
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
	VkPipelineLayout layout;
	_renderpass* renderPass;
	uint32_t subpass;
} _pipeline;

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

	VkRect2D renderArea;
	_renderpass* renderpass;
	_framebuffer* fbo;
	uint32_t currentSubpass;
	_pipeline* graphicsPipeline;
	_pipeline* computePipeline;

	uint32_t vertexBufferOffsets[8];
	_buffer* vertexBuffers[8];
} _commandBuffer;

void getPaddedTextureDimensionsT(uint32_t width, uint32_t height, uint32_t bpp, uint32_t* paddedWidth, uint32_t* paddedHeight);
uint32_t getFormatBpp(VkFormat f);
uint32_t packVec4IntoABGR8(const float rgba[4]);
int findInstanceExtension(char* name);
int findDeviceExtension(char* name);
void createImageBO(_image* i);
