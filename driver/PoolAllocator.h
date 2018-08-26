#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "CustomAssert.h"

#include <stdint.h>

typedef struct PoolAllocator
{
	char* buf; //preallocated buffer
	uint32_t* nextFreeBlock;
	unsigned blockSize;
	unsigned size; //size is exact multiple of block size
} PoolAllocator;

PoolAllocator createPoolAllocator(char* b, unsigned bs, unsigned s);
void destroyPoolAllocator(PoolAllocator* pa);
void* poolAllocate(PoolAllocator* pa);
void poolFree(PoolAllocator* pa, void* p);

#if defined (__cplusplus)
}
#endif
