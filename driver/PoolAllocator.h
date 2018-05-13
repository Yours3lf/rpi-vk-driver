#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "CustomAssert.h"

#include <stdint.h>

typedef struct PoolAllocator
{
	char* buf; //preallocated buffer
	char* nextFreeBlock;
	unsigned blockSize;
	unsigned size; //size is exact multiple of block size
} PoolAllocator;

PoolAllocator createPoolAllocator(char* b, unsigned bs, unsigned s)
{
	assert(b); //only allocated memory
	assert(bs >= sizeof(void*)); //we need to be able to store
	assert(s%bs==0); //we want a size that is the exact multiple of block size
	assert(s > bs); //at least 1 element

	PoolAllocator pa =
	{
		.buf = b,
		.nextFreeBlock = b,
		.blockSize = bs,
		.size = s
	};

	//initialize linked list of free pointers
	char* ptr = pa.nextFreeBlock;
	for(unsigned c = 0; c < s/bs - 1; ++c)
	{
		*(uint32_t*)ptr = ptr + bs;
		ptr += bs;
	}

	*(uint32_t*)ptr = 0; //last element

	return pa;
}

void destroyPoolAllocator(PoolAllocator* pa)
{
	//actual memory freeing is done by caller
	pa->buf = 0;
	pa->nextFreeBlock = 0;
	pa->blockSize = 0;
	pa->size = 0;
}

void* poolAllocte(PoolAllocator* pa)
{
	assert(pa->buf);

	if(!pa->nextFreeBlock)
	{
		return 0; //no free blocks
	}

	//next free block will be allocated
	void* ret = pa->nextFreeBlock;

	//set next free block to the one the current next points to
	pa->nextFreeBlock = *(uint32_t*)pa->nextFreeBlock;

	return ret;
}

void poolFree(PoolAllocator* pa, void* p)
{
	assert(pa->buf);
	assert(p);

	//set block to be freed to point to the current next free block
	*(uint32_t*)p = pa->nextFreeBlock;

	//set next free block to the freshly freed block
	pa->nextFreeBlock = p;
}

#if defined (__cplusplus)
}
#endif
