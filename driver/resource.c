#include "common.h"

#include "declarations.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImageView
 */
VkResult RPIFUNC(vkCreateImageView)(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	PROFILESTART(RPIFUNC(vkCreateImageView));

	assert(device);
	assert(pCreateInfo);
	assert(pView);

	_imageView* view = ALLOCATE(sizeof(_imageView), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!view)
	{
		PROFILEEND(RPIFUNC(vkCreateImageView));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	view->image = pCreateInfo->image;
	view->viewType = pCreateInfo->viewType;
	view->interpretedFormat = pCreateInfo->format;
	view->swizzle = pCreateInfo->components;
	view->subresourceRange = pCreateInfo->subresourceRange;

	*pView = view;

	PROFILEEND(RPIFUNC(vkCreateImageView));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBuffer
 */
VkResult RPIFUNC(vkCreateBuffer)(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	PROFILESTART(RPIFUNC(vkCreateBuffer));

	assert(device);
	assert(pCreateInfo);
	assert(pBuffer);

	_buffer* buf = ALLOCATE(sizeof(_buffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!buf)
	{
		PROFILEEND(RPIFUNC(vkCreateBuffer));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	buf->size = pCreateInfo->size;
	buf->usage = pCreateInfo->usage;
	buf->boundMem = 0;
	buf->alignment = ARM_PAGE_SIZE;
	buf->alignedSize = getBOAlignedSize(buf->size, ARM_PAGE_SIZE);

	*pBuffer = buf;

	PROFILEEND(RPIFUNC(vkCreateBuffer));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetBufferMemoryRequirements
 */
void RPIFUNC(vkGetBufferMemoryRequirements)(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	PROFILESTART(RPIFUNC(vkGetBufferMemoryRequirements));

	assert(device);
	assert(buffer);
	assert(pMemoryRequirements);

	pMemoryRequirements->alignment = ((_buffer*)buffer)->alignment;
	pMemoryRequirements->size = getBOAlignedSize(((_buffer*)buffer)->size, ARM_PAGE_SIZE);
	//there's only one memory type so that's gonna be it...
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	PROFILEEND(RPIFUNC(vkGetBufferMemoryRequirements));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetBufferMemoryRequirements2)(
	VkDevice                                    device,
	const VkBufferMemoryRequirementsInfo2*      pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements)
{
	PROFILESTART(RPIFUNC(vkGetBufferMemoryRequirements2));

	assert(device);
	assert(pInfo);
	assert(pMemoryRequirements);

	RPIFUNC(vkGetBufferMemoryRequirements)(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements);

	PROFILEEND(RPIFUNC(vkGetBufferMemoryRequirements2));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindBufferMemory
 */
VkResult RPIFUNC(vkBindBufferMemory)(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	PROFILESTART(RPIFUNC(vkBindBufferMemory));

	assert(device);
	assert(buffer);
	assert(memory);

	_buffer* buf = buffer;
	_deviceMemory* mem = memory;

	assert(!buf->boundMem);
	assert(memoryOffset < mem->size);
	//assert(memoryOffset % buf->alignment == 0);
	assert(buf->alignedSize <= mem->size - memoryOffset);

	buf->boundMem = mem;
	buf->boundOffset = memoryOffset;

	PROFILEEND(RPIFUNC(vkBindBufferMemory));
	return VK_SUCCESS;
}

void RPIFUNC(vkDestroyBuffer)(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyBuffer));

	assert(device);

	_buffer* buf = buffer;
	if(buf)
	{
		FREE(buf);
	}

	PROFILEEND(RPIFUNC(vkDestroyBuffer));
}

void RPIFUNC(vkDestroyImageView)(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyImageView));

	assert(device);

	_imageView* view = imageView;
	if(view)
	{
		FREE(view);
	}

	PROFILEEND(RPIFUNC(vkDestroyImageView));
}


/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBufferView
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateBufferView)(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView)
{
	PROFILESTART(RPIFUNC(vkCreateBufferView));

	assert(device);
	assert(pCreateInfo);
	assert(pView);

	_bufferView* bv = ALLOCATE(sizeof(_bufferView), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!bv)
	{
		PROFILEEND(RPIFUNC(vkCreateBufferView));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	bv->buffer = pCreateInfo->buffer;
	bv->format = pCreateInfo->format;
	bv->offset = pCreateInfo->offset;
	bv->range = pCreateInfo->range;

	*pView = bv;

	PROFILEEND(RPIFUNC(vkCreateBufferView));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyBufferView
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyBufferView)(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyBufferView));

	assert(device);

	_bufferView* bv = bufferView;
	if(bv)
	{
		FREE(bv);
	}

	PROFILEEND(RPIFUNC(vkDestroyBufferView));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImage
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateImage)(
	VkDevice                                    device,
	const VkImageCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImage*                                    pImage)
{
	PROFILESTART(RPIFUNC(vkCreateImage));

	assert(device);
	assert(pCreateInfo);
	assert(pImage);

	_image* i = ALLOCATE(sizeof(_image), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!i)
	{
		PROFILEEND(RPIFUNC(vkCreateImage));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	i->flags = pCreateInfo->flags;
	i->type = pCreateInfo->imageType;
	i->fb = 0; //needed for modeset
	i->width = pCreateInfo->extent.width;
	i->height = pCreateInfo->extent.height;
	i->depth = pCreateInfo->extent.depth;
	i->miplevels = pCreateInfo->mipLevels;
	memset(i->levelOffsets, 0, sizeof(uint32_t) * 11);
	memset(i->levelTiling, 0, sizeof(uint32_t) * 11);
	i->samples = pCreateInfo->samples;
	i->layers = pCreateInfo->arrayLayers;
	i->size = 0;
	i->stride = 0;
	i->usageBits = pCreateInfo->usage;
	i->format = pCreateInfo->format;
	i->imageSpace = 0;
	i->tiling = 0;
	if(pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR)
	{
		i->tiling = VC4_TILING_FORMAT_LINEAR;
	}
	else
	{
		uint32_t isLT = isLTformat(getFormatBpp(i->format), i->width, i->height);
		if(!isLT)
		{
			 i->tiling = VC4_TILING_FORMAT_T;
		}
		else
		{
			i->tiling = VC4_TILING_FORMAT_LT;
		}
	}
	i->layout = pCreateInfo->initialLayout;
	i->boundMem = 0;
	i->boundOffset = 0;
	i->alignment = ARM_PAGE_SIZE;

	i->concurrentAccess = pCreateInfo->sharingMode; //TODO?
	i->numQueueFamiliesWithAccess = pCreateInfo->queueFamilyIndexCount;
	if(i->numQueueFamiliesWithAccess > 0)
	{
		i->queueFamiliesWithAccess = ALLOCATE(sizeof(uint32_t) * i->numQueueFamiliesWithAccess, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!i->queueFamiliesWithAccess)
		{
			PROFILEEND(RPIFUNC(vkCreateImage));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		memcpy(i->queueFamiliesWithAccess, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t) * i->numQueueFamiliesWithAccess);
	}

	i->preTransformMode = 0;
	i->compositeAlpha = 0;
	i->presentMode = 0;
	i->clipped = 0;

	*pImage = i;

	PROFILEEND(RPIFUNC(vkCreateImage));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyImage
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyImage)(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyImage));

	assert(device);

	_image* i = image;

	if(i)
	{
		if(i->numQueueFamiliesWithAccess > 0)
		{
			FREE(i->queueFamiliesWithAccess);
		}
		FREE(i);
	}

	PROFILEEND(RPIFUNC(vkDestroyImage));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetImageMemoryRequirements
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageMemoryRequirements)(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements)
{
	PROFILESTART(RPIFUNC(vkGetImageMemoryRequirements));

	assert(device);
	assert(image);
	assert(pMemoryRequirements);

	_image* i = image;

	uint32_t bpp = getFormatBpp(i->format);
	uint32_t utileW, utileH;
	getUTileDimensions(bpp, &utileW, &utileH);

	uint32_t w = i->width;
	uint32_t h = i->height;

	if(i->format == VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
	{
		w = (w + 3) >> 2;
		h = (h + 3) >> 2;
	}

	uint32_t potW = getPow2Pad(w);
	uint32_t potH = getPow2Pad(h);
	uint32_t offset = 0;

	uint32_t sizes[11];
	uint32_t strides[11];

	for(int c = i->miplevels - 1; c >= 0; c--)
	{
		i->levelTiling[c] = i->tiling;

		uint32_t mipW, mipH;
		if(!c)
		{
			mipW = w;
			mipH = h;
		}
		else
		{
			mipW = max(potW >> c, 1);
			mipH = max(potH >> c, 1);
		}

		if(i->tiling == VC4_TILING_FORMAT_LINEAR)
		{
			if(i->samples > 1)
			{
				mipW = roundUp(mipW, 32);
				mipH = roundUp(mipH, 32);
			}
			else
			{
				mipW = roundUp(mipW, utileW);
			}
		}
		else
		{
			uint32_t isMipLT = isLTformat(bpp, mipW, mipH);
			if(isMipLT)
			{
				i->levelTiling[c] = VC4_TILING_FORMAT_LT;
				mipW = roundUp(mipW, utileW);
				mipH = roundUp(mipH, utileH);
			}
			else
			{
				mipW = roundUp(mipW, utileW * 8);
				mipH = roundUp(mipH, utileH * 8);
			}
		}

		i->levelOffsets[c] = offset;

		strides[c] = (mipW * bpp * max(i->samples, 1)) >> 3;
		sizes[c] = mipH * strides[c];

		offset += sizes[c];
	}

	uint32_t levelZeroOffset = roundUp(i->levelOffsets[0], ARM_PAGE_SIZE) - i->levelOffsets[0];

	if(levelZeroOffset)
	{
		for(uint32_t c = 0; c < i->miplevels; ++c)
		{
			i->levelOffsets[c] += levelZeroOffset;
		}
	}

	i->size = getBOAlignedSize(sizes[0] + i->levelOffsets[0], ARM_PAGE_SIZE) * i->layers;
	i->stride = strides[0];

	pMemoryRequirements->alignment = ARM_PAGE_SIZE;
	pMemoryRequirements->memoryTypeBits = memoryTypes[0].propertyFlags; //TODO
	pMemoryRequirements->size = i->size;

	PROFILEEND(RPIFUNC(vkGetImageMemoryRequirements));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindImageMemory
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindImageMemory)(
	VkDevice                                    device,
	VkImage                                     image,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset)
{
	PROFILESTART(RPIFUNC(vkBindImageMemory));

	assert(device);
	assert(image);
	assert(memory);

	_image* i = image;
	_deviceMemory* m = memory;

	assert(!i->boundMem);
	assert(memoryOffset < m->size);
	//assert(memoryOffset % i->alignment == 0);
	assert(i->size <= m->size - memoryOffset);

	i->boundMem = m;
	i->boundOffset = memoryOffset;

	//TODO this is necessary, but maybe don't do it here?
	if(i->tiling == VC4_TILING_FORMAT_LINEAR)
	{
		int ret = vc4_bo_set_tiling(controlFd, i->boundMem->bo, DRM_FORMAT_MOD_LINEAR); assert(ret);
	}
	else if(i->tiling == VC4_TILING_FORMAT_T || i->tiling == VC4_TILING_FORMAT_LT)
	{
		int ret = vc4_bo_set_tiling(controlFd, i->boundMem->bo, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED); assert(ret);
	}

	PROFILEEND(RPIFUNC(vkBindImageMemory));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindBufferMemory2)(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindBufferMemoryInfo*               pBindInfos)
{
	PROFILESTART(RPIFUNC(vkBindBufferMemory2));

	assert(device);
	assert(pBindInfos);

	VkResult ret = VK_SUCCESS;

	for(uint32_t c = 0; c < bindInfoCount; ++c)
	{
		VkResult res = RPIFUNC(vkBindBufferMemory)(device, pBindInfos[c].buffer, pBindInfos[c].memory, pBindInfos[c].memoryOffset);
		if(res != VK_SUCCESS)
		{
			ret = res;
		}
	}

	PROFILEEND(RPIFUNC(vkBindBufferMemory2));
	return ret;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageMemoryRequirements2)(
	VkDevice                                    device,
	const VkImageMemoryRequirementsInfo2*       pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements)
{
	PROFILESTART(RPIFUNC(vkGetImageMemoryRequirements2));

	assert(device);
	assert(pInfo);
	assert(pMemoryRequirements);
	RPIFUNC(vkGetImageMemoryRequirements)(device, pInfo->image, pMemoryRequirements);

	PROFILEEND(RPIFUNC(vkGetImageMemoryRequirements2));
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindImageMemory2)(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindImageMemoryInfo*                pBindInfos)
{
	PROFILESTART(RPIFUNC(vkBindImageMemory2));

	assert(device);
	assert(pBindInfos);

	VkResult ret = VK_SUCCESS;

	for(uint32_t c = 0; c < bindInfoCount; ++c)
	{
		VkResult res = RPIFUNC(vkBindImageMemory)(device, pBindInfos[c].image, pBindInfos[c].memory, pBindInfos[c].memoryOffset);
		if(res != VK_SUCCESS)
		{
			ret = res;
		}
	}

	PROFILEEND(RPIFUNC(vkBindImageMemory2));
	return ret;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdPushConstants)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineLayout                            layout,
	VkShaderStageFlags                          stageFlags,
	uint32_t                                    offset,
	uint32_t                                    size,
	const void*                                 pValues)
{
	PROFILESTART(RPIFUNC(vkCmdPushConstants));

	assert(commandBuffer);
	assert(layout);

	_pipelineLayout* pl = layout;
	_commandBuffer* cb = commandBuffer;

	if(stageFlags & VK_SHADER_STAGE_VERTEX_BIT)
	{
		memcpy(cb->pushConstantBufferVertex + offset, pValues, size);
	}

	if(stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		memcpy(cb->pushConstantBufferPixel + offset, pValues, size);
	}

	cb->pushConstantDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdPushConstants));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageSubresourceLayout)(
	VkDevice                                    device,
	VkImage                                     image,
	const VkImageSubresource*                   pSubresource,
	VkSubresourceLayout*                        pLayout)
{
	PROFILESTART(RPIFUNC(vkGetImageSubresourceLayout));

	//TODO

	PROFILEEND(RPIFUNC(vkGetImageSubresourceLayout));
}
