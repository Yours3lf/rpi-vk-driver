#include "ConsecutivePoolAllocator.h"

#include "CustomAssert.h"

#include <stdint.h>
#include <string.h>

ConsecutivePoolAllocator createConsecutivePoolAllocator(char* b, unsigned bs, unsigned s)
{
	assert(b); //only allocated memory
	assert(bs >= sizeof(void*)); //we need to be able to store
	assert(s%bs==0); //we want a size that is the exact multiple of block size
	assert(s >= bs); //at least 1 element

	ConsecutivePoolAllocator pa =
	{
		.buf = b,
		.nextFreeBlock = (uint32_t*)b,
		.blockSize = bs,
		.size = s
	};

	//initialize linked list of free pointers
	uint32_t* ptr = pa.nextFreeBlock;
	unsigned last = s/bs - 1;
	for(unsigned c = 0; c < last; ++c)
	{
		*ptr = (char*)ptr + bs;
		ptr = (char*)ptr + bs;
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
//return an offset into the pool buffer, as pool could be reallocated!
uint32_t consecutivePoolAllocate(ConsecutivePoolAllocator* pa, uint32_t numBlocks)
{
	assert(pa);
	assert(pa->buf);
	assert(numBlocks);

	//CPAdebugPrint(pa);

	uint32_t* ptr = pa->nextFreeBlock;

	if(!ptr)
	{
		return -1; //no free blocks
	}

	//TODO change strategy to search for the blocks that are located closest to the start of the buffer
	//but still are big enough in size
	for(; ptr; ptr = *ptr)
	{
		uint32_t found = 1;
		char* nextBlock = (char*)ptr + pa->blockSize;
		uint32_t* nextFree = *ptr;
		for(uint32_t c = 1; c != numBlocks; ++c)
		{
			if(nextBlock == nextFree)
			{
				nextFree = *nextFree;
				nextBlock += pa->blockSize;
			}
			else
			{
				found = 0;
				break;
			}
		}

		if(found)
		{
			//set the next free block to the one that the last block we allocated points to
			pa->nextFreeBlock = *(uint32_t*)((char*)ptr + (numBlocks - 1) * pa->blockSize);
			break;
		}

		if(!(*ptr))
		{
			return -1;
		}
	}

	//TODO debug stuff, not for release
	if(ptr) memset(ptr, 0, numBlocks * pa->blockSize);

	return (char*)ptr - pa->buf;
}

//free numBlocks consecutive memory
void consecutivePoolFree(ConsecutivePoolAllocator* pa, void* p, uint32_t numBlocks)
{
	assert(pa);
	assert(pa->buf);
	assert(p);
	assert(numBlocks);

	//TODO debug stuff, not for release
	memset(p, 0, numBlocks * pa->blockSize);

	//if linked list of free entries is empty
	if(!pa->nextFreeBlock)
	{
		//then restart linked list
		pa->nextFreeBlock = p;
		char* listPtr = pa->nextFreeBlock;
		for(uint32_t c = 0; c < numBlocks - 1; ++c)
		{
			*(uint32_t*)listPtr = listPtr + pa->blockSize;
			listPtr += pa->blockSize;
		}

		//end list
		*(uint32_t*)listPtr = 0;
	}
	else
	{
		//if list is not empty, try to form consecutive parts

		//search free list to see if the freed element fits anywhere
		uint32_t found = 0;
		for(uint32_t* listPtr = pa->nextFreeBlock; listPtr; listPtr = *listPtr)
		{
			//if the freed block fits in the list somewhere
			if(((char*)listPtr + pa->blockSize) == p)
			{
				//add it into the list
				uint32_t* tmp = *listPtr;
				*listPtr = p;

				//reconstruct linked list within the freed element
				char* ptr = *listPtr;
				for(uint32_t c = 0; c < numBlocks - 1; ++c)
				{
					*(uint32_t*)ptr = ptr + pa->blockSize;
					ptr += pa->blockSize;
				}

				//set the last element to point to the one after
				*(uint32_t*)ptr = tmp;

				found = 1;
			}
		}

		if(!found)
		{
			//if it doesn't fit anywhere, just simply add it to the linked list
			uint32_t* tmp = pa->nextFreeBlock;

			pa->nextFreeBlock = p;
			char* listPtr = pa->nextFreeBlock;
			for(uint32_t c = 0; c < numBlocks - 1; ++c)
			{
				*(uint32_t*)listPtr = listPtr + pa->blockSize;
				listPtr += pa->blockSize;
			}

			//set the last element to point to the one after
			*(uint32_t*)listPtr = tmp;
		}
	}
}

uint32_t consecutivePoolReAllocate(ConsecutivePoolAllocator* pa, void* currentMem, uint32_t currNumBlocks)
{
	assert(pa);
	assert(pa->buf);
	assert(currentMem);
	assert(currNumBlocks);

	uint32_t* nextCandidate = (char*)currentMem + pa->blockSize * currNumBlocks;

	uint32_t* prevPtr = 0;
	for(uint32_t* listPtr = pa->nextFreeBlock; listPtr; listPtr = *listPtr)
	{
		if(listPtr == nextCandidate)
		{
			//update next free block to be the one after our current candidate
			if(prevPtr)
			{
				*prevPtr = *listPtr;
				pa->nextFreeBlock = prevPtr;
			}
			else if(*listPtr)
			{
				pa->nextFreeBlock = *listPtr;
			}

			return (char*)currentMem - pa->buf;
		}

		prevPtr = listPtr;
	}

	{
		//try to allocate one more block
		uint32_t newMemOffset = consecutivePoolAllocate(pa, currNumBlocks + 1);

		if(newMemOffset == -1)
		{
			return -1;
		}
		//copy over old content
		memcpy(pa->buf + newMemOffset, currentMem, currNumBlocks * pa->blockSize);
		//free current element
		consecutivePoolFree(pa, currentMem, currNumBlocks);

		return newMemOffset;
	}
}

void* getCPAptrFromOffset(ConsecutivePoolAllocator* pa, uint32_t offset)
{
	assert(pa);
	assert(pa->buf);
	assert(offset < pa->size);

	return pa->buf + offset;
}

void CPAdebugPrint(ConsecutivePoolAllocator* pa)
{
	fprintf(stderr, "\nCPA Debug Print\n");
	fprintf(stderr, "pa->buf %p\n", pa->buf);
	fprintf(stderr, "pa->nextFreeBlock %p\n", pa->nextFreeBlock);

	fprintf(stderr, "Linear walk:\n");
	for(char* ptr = pa->buf; ptr != pa->buf + pa->size; ptr += pa->blockSize)
	{
		fprintf(stderr, "%p: %p, ", ptr, *(uint32_t*)ptr);
	}

	fprintf(stderr, "\nLinked List walk:\n");
	for(uint32_t* ptr = pa->nextFreeBlock; ptr; ptr = *ptr)
	{
		fprintf(stderr, "%p: %p, ", ptr, *ptr);
	}
	fprintf(stderr, "\n");
}
