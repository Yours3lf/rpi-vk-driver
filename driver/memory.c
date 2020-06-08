#include "common.h"

#include "declarations.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceMemoryProperties
 */
void RPIFUNC(vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
	PROFILESTART(RPIFUNC(vkGetPhysicalDeviceMemoryProperties));

	assert(physicalDevice);
	assert(pMemoryProperties);

	if(memoryHeaps[0].size == 0)
	{
		//TODO is this the correct way of getting amount of video mem?
		char buf[4096];
		int fd = open("/proc/meminfo", O_RDONLY);
		read(fd, buf, 4096);
		close(fd);
		char* cma = strstr(buf, "CmaTotal");
		char* cmaend = strstr(cma, "\n");
		char cmaAmount[4096];
		char* cmaPtr = cmaAmount;
		while(cma != cmaend)
		{
			if(*cma >= '0' && *cma <= '9')
			{
				//number
				*cmaPtr = *cma; //copy char
				cmaPtr++;
			}

			cma++;
		}
		*cmaPtr = '\0';
		unsigned amount = atoi(cmaAmount);
		//printf("%i\n", amount);

		//all heaps share the same memory
		for(uint32_t c = 0; c < numMemoryHeaps; ++c)
		{
			memoryHeaps[c].size = amount * 1000; //kB to B
		}
	}

	pMemoryProperties->memoryTypeCount = numMemoryTypes;
	for(uint32_t c = 0; c < numMemoryTypes; ++c)
	{
		pMemoryProperties->memoryTypes[c] = memoryTypes[c];
	}

	pMemoryProperties->memoryHeapCount = numMemoryHeaps;
	for(uint32_t c = 0; c < numMemoryHeaps; ++c)
	{
		pMemoryProperties->memoryHeaps[c] = memoryHeaps[c];
	}

	PROFILEEND(RPIFUNC(vkGetPhysicalDeviceMemoryProperties));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAllocateMemory
 */
VkResult RPIFUNC(vkAllocateMemory)(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
	PROFILESTART(RPIFUNC(vkAllocateMemory));

	assert(device);
	assert(pAllocateInfo);
	assert(pMemory);

	uint32_t bo = vc4_bo_alloc(controlFd, pAllocateInfo->allocationSize, "vkAllocateMemory");
	if(!bo)
	{
		PROFILEEND(RPIFUNC(vkAllocateMemory));
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	_deviceMemory* mem = ALLOCATE(sizeof(_deviceMemory), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!mem)
	{
		PROFILEEND(RPIFUNC(vkAllocateMemory));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	mem->bo = bo;
	mem->size = pAllocateInfo->allocationSize;
	mem->memTypeIndex = pAllocateInfo->memoryTypeIndex;
	mem->mappedPtr = 0;

	*pMemory = mem;

	//TODO max number of allocations

	PROFILEEND(RPIFUNC(vkAllocateMemory));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkMapMemory
 */
VkResult RPIFUNC(vkMapMemory)(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
	PROFILESTART(RPIFUNC(vkMapMemory));

	assert(device);
	assert(memory);
	assert(size);
	assert(ppData);

	assert(memoryTypes[((_deviceMemory*)memory)->memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	assert(!((_deviceMemory*)memory)->mappedPtr);
	assert(offset < ((_deviceMemory*)memory)->size);
	if(size != VK_WHOLE_SIZE)
	{
		assert(size > 0);
		assert(size <= ((_deviceMemory*)memory)->size - offset);
	}
	else
	{
		size = ((_deviceMemory*)memory)->size;
	}

	void* ptr = vc4_bo_map(controlFd, ((_deviceMemory*)memory)->bo, offset, size);
	if(!ptr)
	{
		PROFILEEND(RPIFUNC(vkMapMemory));
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	((_deviceMemory*)memory)->mappedPtr = ptr;
	((_deviceMemory*)memory)->mappedOffset = offset;
	((_deviceMemory*)memory)->mappedSize = size;
	*ppData = ptr;

	PROFILEEND(RPIFUNC(vkMapMemory));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkUnmapMemory
 */
void RPIFUNC(vkUnmapMemory)(VkDevice device, VkDeviceMemory memory)
{
	PROFILESTART(RPIFUNC(vkUnmapMemory));

	assert(device);
	assert(memory);

	vc4_bo_unmap_unsynchronized(controlFd, ((_deviceMemory*)memory)->mappedPtr, ((_deviceMemory*)memory)->mappedSize);
	((_deviceMemory*)memory)->mappedPtr = 0;
	((_deviceMemory*)memory)->mappedSize = 0;
	((_deviceMemory*)memory)->mappedOffset = 0;

	PROFILEEND(RPIFUNC(vkUnmapMemory));
}

void RPIFUNC(vkFreeMemory)(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
	PROFILESTART(RPIFUNC(vkFreeMemory));

	assert(device);

	_deviceMemory* mem = memory;
	if(mem)
	{
		vc4_set_madvise(controlFd, mem->bo, 0, device->dev->instance->hasMadvise);
		vc4_bo_free(controlFd, mem->bo, mem->mappedPtr, mem->size);
		FREE(mem);
	}

	PROFILEEND(RPIFUNC(vkFreeMemory));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkFlushMappedMemoryRanges
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkFlushMappedMemoryRanges)(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges)
{
	PROFILESTART(RPIFUNC(vkFlushMappedMemoryRanges));

	//TODO

	PROFILEEND(RPIFUNC(vkFlushMappedMemoryRanges));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkInvalidateMappedMemoryRanges
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkInvalidateMappedMemoryRanges)(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges)
{
	//TODO

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceMemoryProperties2)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
	assert(physicalDevice);
	RPIFUNC(vkGetPhysicalDeviceMemoryProperties)(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDeviceMemoryCommitment)(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize*                               pCommittedMemoryInBytes)
{
	//TODO
}
