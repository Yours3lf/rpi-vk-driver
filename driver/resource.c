#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImageView
 */
VkResult vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
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

	//TODO errors/validation

	*pView = view;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBuffer
 */
VkResult vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
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
	buf->alignment = ARM_PAGE_SIZE; //TODO
	buf->alignedSize = getBOAlignedSize(buf->size);

	*pBuffer = buf;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetBufferMemoryRequirements
 */
void vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	assert(device);
	assert(buffer);
	assert(pMemoryRequirements);

	pMemoryRequirements->alignment = ((_buffer*)buffer)->alignment;
	pMemoryRequirements->size = ((_buffer*)buffer)->alignedSize;
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; //TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindBufferMemory
 */
VkResult vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
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

void vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(buffer);

	_buffer* buf = buffer;
	FREE(buf);
}

void vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(imageView);

	_imageView* view = imageView;
	FREE(view);
}


/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBufferView
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	_bufferView* bv = ALLOCATE(sizeof(_bufferView), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	bv->buffer = pCreateInfo->buffer;
	bv->format = pCreateInfo->format;
	bv->offset = pCreateInfo->offset;
	bv->range = pCreateInfo->range;

	*pView = bv;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyBufferView
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(bufferView);

	_bufferView* bv = bufferView;
	FREE(bv);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImage
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(
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

	//TODO flags?
	i->type = pCreateInfo->imageType;
	i->fb = 0; //needed for modeset
	i->width = pCreateInfo->extent.width;
	i->height = pCreateInfo->extent.height;
	i->depth = pCreateInfo->extent.depth;
	i->paddedWidth = 0; //when format is T
	i->paddedHeight = 0;
	i->miplevels = pCreateInfo->mipLevels;
	i->samples = pCreateInfo->samples;
	i->layers = pCreateInfo->arrayLayers;
	i->size = 0;
	i->stride = 0;
	i->usageBits = pCreateInfo->usage;
	i->format = pCreateInfo->format;
	i->imageSpace = 0;
	i->tiling = pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR ? VC4_TILING_FORMAT_LT : VC4_TILING_FORMAT_T;
	i->needToClear = 0;
	i->clearColor[0] = i->clearColor[1] = 0;
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
			return VK_ERROR_OUT_OF_HOST_MEMORY;
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
VKAPI_ATTR void VKAPI_CALL vkDestroyImage(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(image);

	_image* i = image;

	if(i->numQueueFamiliesWithAccess > 0);
		FREE(i->queueFamiliesWithAccess);

	FREE(i);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetImageMemoryRequirements
 */
VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements)
{
	assert(device);
	assert(image);
	assert(pMemoryRequirements);

	_image* i = image;

	uint32_t bpp = getFormatBpp(i->format);
	uint32_t pixelSizeBytes = bpp / 8;
	uint32_t nonPaddedSize = i->width * i->height * pixelSizeBytes;
	i->paddedWidth = i->width;
	i->paddedHeight = i->height;

	//need to pad to T format, as HW automatically chooses that
	if(nonPaddedSize > 4096)
	{
		getPaddedTextureDimensionsT(i->width, i->height, bpp, &i->paddedWidth, &i->paddedHeight);
	}

	i->size = getBOAlignedSize(i->paddedWidth * i->paddedHeight * pixelSizeBytes);
	i->stride = i->paddedWidth * pixelSizeBytes;

	pMemoryRequirements->alignment = ARM_PAGE_SIZE;
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; //TODO
	pMemoryRequirements->size = i->size;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindImageMemory
 */
VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(
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

	i->boundMem = m;
	i->boundOffset = memoryOffset;

	return VK_SUCCESS;
}
