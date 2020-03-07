#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include "driver/CustomAssert.h"
#include "driver/ConsecutivePoolAllocator.h"

#include <vulkan/vulkan.h>

#include "driver/vkExt.h"

void simpleTest()
{
	uint32_t blocksize = 16;
	uint32_t numblocks = 8;
	uint32_t size = numblocks * blocksize;

	ConsecutivePoolAllocator cpa = createConsecutivePoolAllocator((char*)malloc(size), blocksize, size);
	CPAdebugPrint(&cpa);

	void* mem1 = consecutivePoolAllocate(&cpa, 1);
	CPAdebugPrint(&cpa);

	void* mem2 = consecutivePoolAllocate(&cpa, 2);
	CPAdebugPrint(&cpa);

	void* mem3 = consecutivePoolAllocate(&cpa, 3);
	CPAdebugPrint(&cpa);

	void* mem11 = consecutivePoolAllocate(&cpa, 1);
	CPAdebugPrint(&cpa);

	void* mem111 = consecutivePoolAllocate(&cpa, 1);
	CPAdebugPrint(&cpa);

	void* mem0 = consecutivePoolAllocate(&cpa, 1);
	fprintf(stderr, "\n%p\n", mem0);

	consecutivePoolFree(&cpa,  mem11, 1);
	CPAdebugPrint(&cpa);

	consecutivePoolFree(&cpa,  mem111, 1);
	CPAdebugPrint(&cpa);

	consecutivePoolFree(&cpa,  mem2, 2);
	CPAdebugPrint(&cpa);

	consecutivePoolFree(&cpa,  mem3, 3);
	CPAdebugPrint(&cpa);

	consecutivePoolFree(&cpa,  mem1, 1);
	CPAdebugPrint(&cpa);
}

void allocTest(uint32_t numToAlloc)
{
	uint32_t blocksize = 16;
	uint32_t numblocks = 8;
	uint32_t size = numblocks * blocksize;

	ConsecutivePoolAllocator cpa = createConsecutivePoolAllocator((char*)malloc(size), blocksize, size);
	//CPAdebugPrint(&cpa);

	void* mem1 = consecutivePoolAllocate(&cpa, numToAlloc);
	CPAdebugPrint(&cpa);

	fprintf(stderr, "\nmem %p\n", mem1);
}

void freeOneTest(uint32_t which)
{
	uint32_t blocksize = 16;
	uint32_t numblocks = 8;
	uint32_t size = numblocks * blocksize;

	ConsecutivePoolAllocator cpa = createConsecutivePoolAllocator((char*)malloc(size), blocksize, size);
	//CPAdebugPrint(&cpa);

	void* mem[8];
	for(uint32_t c = 0; c < 8; ++c)
	{
		mem[c] = consecutivePoolAllocate(&cpa, 1);
	}

	consecutivePoolFree(&cpa, mem[which], 1);
	CPAdebugPrint(&cpa);

	//fprintf(stderr, "\nmem %p\n", mem);
}

int main() {
	//simpleTest();

	allocTest(1);
	allocTest(3);
	allocTest(8);
	allocTest(9);

	return 0;
}
