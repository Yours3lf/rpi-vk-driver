#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "CustomAssert.h"

#include <stdint.h>

typedef struct LinearAllocator
{
	char* buf; //preallocated buffer
	unsigned offset;
	unsigned size;
} LinearAllocator;

LinearAllocator createLinearAllocator(char* b, unsigned s);
void destroyLinearAllocator(LinearAllocator* la);
void* linearAllocte(LinearAllocator* la, unsigned s);
void linearFree(LinearAllocator* la, void* p);

#if defined (__cplusplus)
}
#endif

