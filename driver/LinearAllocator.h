#pragma once

#include "CustomAssert.h"

#include <stdint.h>

typedef struct LinearAllocator
{
	char* buf; //preallocated buffer
	unsigned offset;
	unsigned size;
} LinearAllocator;

LinearAllocator createLinearAllocator(char* b, unsigned s)
{
	assert(b);
	assert(s > 0);

	LinearAllocator la =
	{
		.buf = b,
		.offset = 0,
		.size = s
	};

	return la;
}

void destroyLinearAllocator(LinearAllocator* la)
{
	la->buf = 0;
	la->offset = 0;
	la->size = 0;
}

void* linearAllocte(LinearAllocator* la, unsigned s)
{
	assert(la->buf);
	assert(la->size > 0);

	if(la->offset + s >= la->size)
	{
		return 0; //no space left
	}

	char* p = la->buf + la->offset + s;
	la->offset += s;

	return p;
}

void linearFree(LinearAllocator* la, void* p)
{
	//assert(0); //this shouldn't really happen, just destroy/reset the whole allocator
}

