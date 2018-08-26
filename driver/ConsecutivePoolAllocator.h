#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "CustomAssert.h"

#include <stdint.h>

typedef struct ConsecutivePoolAllocator
{
	char* buf; //preallocated buffer
	uint32_t* nextFreeBlock;
	unsigned blockSize;
	unsigned size; //size is exact multiple of block size
} ConsecutivePoolAllocator;

ConsecutivePoolAllocator createConsecutivePoolAllocator(char* b, unsigned bs, unsigned s);
void destroyConsecutivePoolAllocator(ConsecutivePoolAllocator* pa);
void* consecutivePoolAllocate(ConsecutivePoolAllocator* pa, uint32_t numBlocks);
void consecutivePoolFree(ConsecutivePoolAllocator* pa, void* p, uint32_t numBlocks);
void* consecutivePoolReAllocate(ConsecutivePoolAllocator* pa, void* currentMem, uint32_t currNumBlocks);

#if defined (__cplusplus)
}
#endif
