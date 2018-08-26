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

#include "modeset.h"
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

void getPaddedTextureDimensionsT(uint32_t width, uint32_t height, uint32_t bpp, uint32_t* paddedWidth, uint32_t* paddedHeight);
uint32_t getFormatBpp(VkFormat f);
uint32_t packVec4IntoABGR8(const float rgba[4]);
int findInstanceExtension(char* name);
int findDeviceExtension(char* name);
void createImageBO(_image* i);
