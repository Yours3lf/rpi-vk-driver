#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImageView
 */
VkResult rpi_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	_imageView* view = ALLOCATE(sizeof(_imageView), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!view)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	view->image = pCreateInfo->image;
	view->viewType = pCreateInfo->viewType;
	view->interpretedFormat = pCreateInfo->format;
	view->swizzle = pCreateInfo->components;
	view->subresourceRange = pCreateInfo->subresourceRange;

	*pView = view;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBuffer
 */
VkResult rpi_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pBuffer);

	_buffer* buf = ALLOCATE(sizeof(_buffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!buf)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	buf->size = pCreateInfo->size;
	buf->usage = pCreateInfo->usage;
	buf->boundMem = 0;
	buf->alignment = ARM_PAGE_SIZE;
	buf->alignedSize = getBOAlignedSize(buf->size, ARM_PAGE_SIZE);

	*pBuffer = buf;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetBufferMemoryRequirements
 */
void rpi_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	assert(device);
	assert(buffer);
	assert(pMemoryRequirements);

	pMemoryRequirements->alignment = ((_buffer*)buffer)->alignment;
	pMemoryRequirements->size = getBOAlignedSize(((_buffer*)buffer)->size, ARM_PAGE_SIZE);
	//there's only one memory type so that's gonna be it...
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetBufferMemoryRequirements2(
	VkDevice                                    device,
	const VkBufferMemoryRequirementsInfo2*      pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements)
{
	assert(device);
	assert(pInfo);
	assert(pMemoryRequirements);

	rpi_vkGetBufferMemoryRequirements(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindBufferMemory
 */
VkResult rpi_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	assert(device);
	assert(buffer);
	assert(memory);

	_buffer* buf = buffer;
	_deviceMemory* mem = memory;

	assert(!buf->boundMem);
	assert(memoryOffset < mem->size);
	assert(memoryOffset % buf->alignment == 0);
	assert(buf->alignedSize <= mem->size - memoryOffset);

	buf->boundMem = mem;
	buf->boundOffset = memoryOffset;

	return VK_SUCCESS;
}

void rpi_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_buffer* buf = buffer;
	if(buf)
	{
		FREE(buf);
	}
}

void rpi_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_imageView* view = imageView;
	if(view)
	{
		FREE(view);
	}
}


/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBufferView
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateBufferView(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	_bufferView* bv = ALLOCATE(sizeof(_bufferView), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!bv)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	bv->buffer = pCreateInfo->buffer;
	bv->format = pCreateInfo->format;
	bv->offset = pCreateInfo->offset;
	bv->range = pCreateInfo->range;

	*pView = bv;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyBufferView
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyBufferView(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	_bufferView* bv = bufferView;
	if(bv)
	{
		FREE(bv);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImage
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateImage(
	VkDevice                                    device,
	const VkImageCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImage*                                    pImage)
{
	assert(device);
	assert(pCreateInfo);
	assert(pImage);

	_image* i = ALLOCATE(sizeof(_image), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!i)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	i->flags = pCreateInfo->flags;
	i->type = pCreateInfo->imageType;
	i->fb = 0; //needed for modeset
	i->width = pCreateInfo->extent.width;
	i->height = pCreateInfo->extent.height;
	i->depth = pCreateInfo->extent.depth;
	i->paddedWidth = 0; //when format is T
	i->paddedHeight = 0;
	i->miplevels = pCreateInfo->mipLevels;
	memset(i->levelOffsets, 0, sizeof(uint32_t) * 11);
	i->samples = pCreateInfo->samples;
	i->layers = pCreateInfo->arrayLayers;
	i->size = 0;
	i->stride = 0;
	i->usageBits = pCreateInfo->usage;
	i->format = pCreateInfo->format;
	i->imageSpace = 0;
	uint32_t nonPaddedSize = (i->width * i->height * getFormatBpp(i->format)) >> 3;
	i->tiling = 0;
	if(pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR)
	{
		i->tiling = VC4_TILING_FORMAT_LINEAR;
	}
	else
	{
		if(nonPaddedSize >= 4096)
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
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		memcpy(i->queueFamiliesWithAccess, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t) * i->numQueueFamiliesWithAccess);
	}

	i->preTransformMode = 0;
	i->compositeAlpha = 0;
	i->presentMode = 0;
	i->clipped = 0;

	*pImage = i;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyImage
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyImage(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator)
{
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
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetImageMemoryRequirements
 */
VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements)
{
	assert(device);
	assert(image);
	assert(pMemoryRequirements);

	_image* i = image;

	uint32_t bpp = getFormatBpp(i->format);
	uint32_t utileW, utileH;
	getUTileDimensions(bpp, &utileW, &utileH);

	switch(i->tiling)
	{
	case VC4_TILING_FORMAT_T:
		{
			//need to pad to T format, as HW automatically chooses that
			i->paddedWidth = roundUp(i->width, utileW * 8);
			i->paddedHeight = roundUp(i->height, utileH * 8);
			break;
		}
	case VC4_TILING_FORMAT_LT:
		{
			//LT format
			i->paddedWidth = roundUp(i->width, utileW);
			i->paddedHeight = roundUp(i->height, utileH);
			break;
		}
	case VC4_TILING_FORMAT_LINEAR:
		{
			//linear format
			i->paddedWidth = roundUp(i->width, utileW);
			i->paddedHeight = i->height;
			break;
		}
	}

	i->stride = (i->paddedWidth * bpp) >> 3;

	//mip levels are laid out in memory the following way:
	//0x0.................................................0xffffff
	//smallest mip level ... largest mip level - 1, base mip level
	//base mip level offset must be a multiple of 4KB
	//mip levels other than the base must be aligned to power of two sizes
	//mip levels must be padded to either T or LT format depending on size

	uint32_t prevMipPaddedSize = 0;
	uint32_t mipSizes[11];

	for(uint32_t c = i->miplevels - 1; c >= 1; --c)
	{
		uint32_t mipWidth = max(i->width >> c, 1);
		uint32_t mipHeight = max(i->height >> c, 1);
		uint32_t nonPaddedSize = (mipWidth * mipHeight * bpp) >> 3;
		uint32_t mipPaddedWidth, mipPaddedHeight;

//		uint32_t tiling = i->tiling;

//		if(i->tiling == VC4_TILING_FORMAT_T && nonPaddedSize <= 4096)
//		{
//			tiling = VC4_TILING_FORMAT_LT;
//		}

//		switch(tiling)
//		{
//		case VC4_TILING_FORMAT_T:
//			{
//				//T format
//				mipPaddedWidth = roundUp(mipWidth, utileW * 8);
//				mipPaddedHeight = roundUp(mipHeight, utileH * 8);
//				break;
//			}
//		case VC4_TILING_FORMAT_LT:
//			{
//				//LT format
//				mipPaddedWidth = roundUp(mipWidth, utileW);
//				mipPaddedHeight = roundUp(mipHeight, utileH);
//				break;
//			}
//		case VC4_TILING_FORMAT_LINEAR:
//			{
//				//linear format
//				mipPaddedWidth = roundUp(mipWidth, utileW);
//				mipPaddedHeight = mipHeight;
//				break;
//			}
//		}

		mipPaddedWidth = getPow2Pad(mipWidth);
		mipPaddedHeight = getPow2Pad(mipHeight);
		uint32_t greater = mipPaddedWidth > mipPaddedHeight ? mipPaddedWidth : mipPaddedHeight;
		greater = greater < 4 ? 4 : greater;
		mipPaddedWidth = mipPaddedHeight = greater;

		uint32_t mipPaddedSize = (mipPaddedWidth * mipPaddedHeight * bpp) >> 3;

		mipSizes[c] = mipPaddedSize;
		i->levelOffsets[c] = prevMipPaddedSize;
		prevMipPaddedSize += mipPaddedSize;

//		fprintf(stderr, "mipPaddedWidth: %u\n", mipPaddedWidth);
//		fprintf(stderr, "mipPaddedHeight: %u\n", mipPaddedHeight);
//		fprintf(stderr, "i->levelOffsets[%u]: %u\n", c, i->levelOffsets[c]);
	}

	//must be a multiple of 4096 bytes
	i->levelOffsets[0] = getBOAlignedSize(prevMipPaddedSize, 4096);

	for(uint32_t c = 1; c < i->miplevels; c++)
	{
		i->levelOffsets[c] = i->levelOffsets[c - 1] - mipSizes[c];
	}

	i->size = getBOAlignedSize(((i->paddedWidth * i->paddedHeight * bpp) >> 3) + i->levelOffsets[0], ARM_PAGE_SIZE) * i->layers;

//	fprintf(stderr, "i->tiling %u\n", i->tiling);
//	fprintf(stderr, "i->levelOffsets[0] %u\n", i->levelOffsets[0]);
//	fprintf(stderr, "i->size %u\n", i->size);
//	fprintf(stderr, "i->paddedWidth %u\n", i->paddedWidth);
//	fprintf(stderr, "i->paddedHeight %u\n", i->paddedHeight);
//	fprintf(stderr, "mipSize %u\n", mipSize);
//	fprintf(stderr, "bpp %u\n", bpp);

	pMemoryRequirements->alignment = ARM_PAGE_SIZE;
	pMemoryRequirements->memoryTypeBits = memoryTypes[0].propertyFlags; //TODO
	pMemoryRequirements->size = i->size;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindImageMemory
 */
VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindImageMemory(
	VkDevice                                    device,
	VkImage                                     image,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset)
{
	assert(device);
	assert(image);
	assert(memory);

	_image* i = image;
	_deviceMemory* m = memory;

	assert(!i->boundMem);
	assert(memoryOffset < m->size);
	assert(memoryOffset % i->alignment == 0);
	assert(i->size <= m->size - memoryOffset);

	//image memory must be aligned to 4096 bytes
	assert((memoryOffset % 4096) == 0);

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

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindBufferMemory2(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindBufferMemoryInfo*               pBindInfos)
{
	assert(device);
	assert(pBindInfos);

	VkResult ret = VK_SUCCESS;

	for(uint32_t c = 0; c < bindInfoCount; ++c)
	{
		VkResult res = rpi_vkBindBufferMemory(device, pBindInfos[c].buffer, pBindInfos[c].memory, pBindInfos[c].memoryOffset);
		if(res != VK_SUCCESS)
		{
			ret = res;
		}
	}

	return ret;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageMemoryRequirements2(
	VkDevice                                    device,
	const VkImageMemoryRequirementsInfo2*       pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements)
{
	assert(device);
	assert(pInfo);
	assert(pMemoryRequirements);
	rpi_vkGetImageMemoryRequirements(device, pInfo->image, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindImageMemory2(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindImageMemoryInfo*                pBindInfos)
{
	assert(device);
	assert(pBindInfos);

	VkResult ret = VK_SUCCESS;

	for(uint32_t c = 0; c < bindInfoCount; ++c)
	{
		VkResult res = rpi_vkBindImageMemory(device, pBindInfos[c].image, pBindInfos[c].memory, pBindInfos[c].memoryOffset);
		if(res != VK_SUCCESS)
		{
			ret = res;
		}
	}

	return ret;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdPushConstants(
	VkCommandBuffer                             commandBuffer,
	VkPipelineLayout                            layout,
	VkShaderStageFlags                          stageFlags,
	uint32_t                                    offset,
	uint32_t                                    size,
	const void*                                 pValues)
{
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
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSubresourceLayout(
	VkDevice                                    device,
	VkImage                                     image,
	const VkImageSubresource*                   pSubresource,
	VkSubresourceLayout*                        pLayout)
{
	//TODO
}
