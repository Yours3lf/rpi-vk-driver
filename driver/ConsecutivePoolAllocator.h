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

ConsecutivePoolAllocator createConsecutivePoolAllocator(char* b, unsigned bs, unsigned s)
{
	assert(b); //only allocated memory
	assert(bs >= sizeof(void*)); //we need to be able to store
	assert(s%bs==0); //we want a size that is the exact multiple of block size
	assert(s > bs); //at least 1 element

	ConsecutivePoolAllocator pa =
	{
		.buf = b,
		.nextFreeBlock = b,
		.blockSize = bs,
		.size = s
	};

	//initialize linked list of free pointers
	uint32_t* ptr = pa.nextFreeBlock;
	for(unsigned c = 0; c < s/bs - 1; ++c)
	{
		*ptr = ptr + bs;
		ptr += bs;
	}

	*ptr = 0; //last element

	return pa;
}

void destroyConsecutivePoolAllocator(ConsecutivePoolAllocator* pa)
{
	//actual memory freeing is done by caller
	pa->buf = 0;
	pa->nextFreeBlock = 0;
	pa->blockSize = 0;
	pa->size = 0;
}

//allocate numBlocks consecutive memory
void* consecutivePoolAllocate(ConsecutivePoolAllocator* pa, uint32_t numBlocks)
{
	assert(pa->buf);

	if(!pa->nextFreeBlock)
	{
		return 0; //no free blocks
	}

	void* ret = 0;
	for(uint32_t* candidate = pa->nextFreeBlock; candidate; candidate = *candidate)
	{
		uint32_t found = 1;
		uint32_t* prevBlock = candidate;
		uint32_t* blockAfterCandidate = *candidate;
		//check if there are enough consecutive free blocks
		for(uint32_t c = 0; c < numBlocks - 1; ++c)
		{
			if(blockAfterCandidate - prevBlock != pa->blockSize)
			{
				//signal if not consecutive (ie. diff is greater than blocksize)
				found = 0;
				break;
			}
			prevBlock = blockAfterCandidate;
			blockAfterCandidate = *blockAfterCandidate;
		}

		//numblocks consecutive blocks found
		if(found)
		{
			ret = candidate;
			if(pa->nextFreeBlock == candidate)
			{
				//candidate found immediately
				pa->nextFreeBlock = blockAfterCandidate;
			}
			else
			{
				//somewhere the linked list would point to candidate, we need to correct this
				for(uint32_t* nextFreeBlockCandidate = pa->nextFreeBlock; nextFreeBlockCandidate; nextFreeBlockCandidate = *nextFreeBlockCandidate)
				{
					if(*nextFreeBlockCandidate == candidate)
					{
						*nextFreeBlockCandidate = blockAfterCandidate;
						break;
					}
				}
			}
			break;
		}
	}

	return ret;
}

//free numBlocks consecutive memory
void consecutivePoolFree(ConsecutivePoolAllocator* pa, void* p, uint32_t numBlocks)
{
	assert(pa->buf);
	assert(p);

	if(pa->nextFreeBlock > p)
	{
		for(uint32_t c = 0; c < numBlocks - 1; ++c)
		{
			//set each allocated block to form a linked list
			*(uint32_t*)((char*)p + c * pa->blockSize) = (char*)p + (c + 1) * pa->blockSize;
		}
		//set last block to point to the next free
		*(uint32_t*)((char*)p + (numBlocks - 1) * pa->blockSize) = pa->nextFreeBlock;
		//set next free to the newly freed block
		pa->nextFreeBlock = p;
		return;
	}

	//somewhere the linked list may point after the free block (or null), we need to correct this
	for(uint32_t* nextFreeBlockCandidate = pa->nextFreeBlock; nextFreeBlockCandidate; nextFreeBlockCandidate = *nextFreeBlockCandidate)
	{
		if(*nextFreeBlockCandidate > p || !*nextFreeBlockCandidate)
		{
			for(uint32_t c = 0; c < numBlocks - 1; ++c)
			{
				//set each allocated block to form a linked list
				*(uint32_t*)((char*)p + c * pa->blockSize) = (char*)p + (c + 1) * pa->blockSize;
			}
			//set last block to point to the next free
			*(uint32_t*)((char*)p + (numBlocks - 1) * pa->blockSize) = *nextFreeBlockCandidate;

			*nextFreeBlockCandidate = p;
			break;
		}
	}
}

//if there's a block free after the current block, it just allocates one more block
//else it frees current block and allocates a new one
void* consecutivePoolReAllocate(ConsecutivePoolAllocator* pa, void* currentMem, uint32_t currNumBlocks)
{
	if(pa->nextFreeBlock == (char*)currentMem + currNumBlocks * pa->blockSize)
	{
		//we have one more block after current one, so just expand current
		pa->nextFreeBlock = *pa->nextFreeBlock;
		return currentMem;
	}
	else
	{
		void* ret = consecutivePoolAllocate(pa, currNumBlocks + 1);
		consecutivePoolFree(pa, currentMem, currNumBlocks);
		return ret;
	}
}

#if defined (__cplusplus)
}
#endif
