#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceMemoryProperties
 */
void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
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
		for(int c = 0; c < numMemoryHeaps; ++c)
		{
			memoryHeaps[c].size = amount * 1000; //kB to B
		}
	}

	pMemoryProperties->memoryTypeCount = numMemoryTypes;
	for(int c = 0; c < numMemoryTypes; ++c)
	{
		pMemoryProperties->memoryTypes[c] = memoryTypes[c];
	}

	pMemoryProperties->memoryHeapCount = numMemoryHeaps;
	for(int c = 0; c < numMemoryHeaps; ++c)
	{
		pMemoryProperties->memoryHeaps[c] = memoryHeaps[c];
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAllocateMemory
 */
VkResult vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
	assert(device);
	assert(pAllocateInfo);
	assert(pMemory);

	uint32_t bo = vc4_bo_alloc(controlFd, pAllocateInfo->allocationSize, "vkAllocateMemory");
	if(!bo)
	{
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	_deviceMemory* mem = ALLOCATE(sizeof(_deviceMemory), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!mem)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	mem->bo = bo;
	mem->size = pAllocateInfo->allocationSize;
	mem->memTypeIndex = pAllocateInfo->memoryTypeIndex;
	mem->mappedPtr = 0;

	*pMemory = mem;

	//TODO max number of allocations

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkMapMemory
 */
VkResult vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
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

	//TODO check ppdata alignment
	//TODO multiple instances?

	void* ptr = vc4_bo_map(controlFd, ((_deviceMemory*)memory)->bo, offset, size);
	if(!ptr)
	{
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	((_deviceMemory*)memory)->mappedPtr = ptr;
	((_deviceMemory*)memory)->mappedOffset = offset;
	((_deviceMemory*)memory)->mappedSize = size;
	*ppData = ptr;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkUnmapMemory
 */
void vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
	assert(device);
	assert(memory);

	vc4_bo_unmap_unsynchronized(controlFd, ((_deviceMemory*)memory)->mappedPtr, ((_deviceMemory*)memory)->mappedSize);
	((_deviceMemory*)memory)->mappedPtr = 0;
	((_deviceMemory*)memory)->mappedSize = 0;
	((_deviceMemory*)memory)->mappedOffset = 0;
}

void vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_deviceMemory* mem = memory;
	if(mem)
	{
		vc4_bo_free(controlFd, mem->bo, mem->mappedPtr, mem->size);
		FREE(mem);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkFlushMappedMemoryRanges
 */
VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges)
{
	//TODO

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkInvalidateMappedMemoryRanges
 */
VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges)
{
	//TODO

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{

}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize*                               pCommittedMemoryInBytes)
{

}
