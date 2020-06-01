#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "CustomAssert.h"

#include "PoolAllocator.h"

#include <stdint.h>

typedef struct FifoElem
{
	struct FifoElem* next;
	struct FifoElem* prev;
	void* data;
} FifoElem;

typedef struct Fifo
{
	FifoElem* first;
	FifoElem* last;
	PoolAllocator dataBuf;
	PoolAllocator fifoElemBuf;
	uint32_t dataSize;
	uint32_t maxElems;
} Fifo;

Fifo createFifo(void* dataMem, void* fifoElemMem, uint32_t maxElems, uint32_t dataSize);
void destroyFifo(Fifo* f);
uint32_t fifoAdd(Fifo* f, void* data);
void fifoRemove(Fifo* f, void* data);
void debugPrintFifo(Fifo* f);

#if defined (__cplusplus)
}
#endif
