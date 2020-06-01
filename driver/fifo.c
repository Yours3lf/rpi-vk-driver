#include "fifo.h"

#if defined (__cplusplus)
extern "C" {
#endif

Fifo createFifo(void* dataMem, void* fifoElemMem, uint32_t maxElems, uint32_t dataSize)
{
	Fifo f;
	f.first = 0;
	f.last = 0;
	f.dataSize = dataSize;
	f.maxElems = maxElems;
	f.dataBuf = createPoolAllocator(dataMem, dataSize, maxElems * dataSize);
	f.fifoElemBuf = createPoolAllocator(fifoElemMem, sizeof(FifoElem), maxElems * sizeof(FifoElem));

	return f;
}

void destroyFifo(Fifo* f)
{
	assert(f);
	f->first = 0;
	f->last = 0;
	destroyPoolAllocator(&f->dataBuf);
	destroyPoolAllocator(&f->fifoElemBuf);
}

uint32_t fifoAdd(Fifo* f, void* data)
{
	assert(f);
	assert(data);

	void* dataPtr = poolAllocate(&f->dataBuf);
	FifoElem* elemPtr = poolAllocate(&f->fifoElemBuf);

	memcpy(dataPtr, data, f->dataSize);
	elemPtr->data = dataPtr;

	if(!dataPtr || !elemPtr)
	{
		return 0;
	}

	if(!f->first)
	{
		elemPtr->prev = 0;
		elemPtr->next = 0;

		f->first = elemPtr;
		f->last = elemPtr;
	}
	else
	{
		FifoElem* tmpFirst = f->first;

		elemPtr->next = tmpFirst;
		elemPtr->prev = 0;
		tmpFirst->prev = elemPtr;

		f->first = elemPtr;
	}

	return 1;
}

void fifoRemove(Fifo* f, void* data)
{
	assert(f);

	if(f->last)
	{
		memcpy(data, f->last->data, f->dataSize);

		FifoElem* tmp = f->last;

		f->last = f->last->prev;

		if(f->last)
		{
			f->last->next = 0;
		}
		else
		{
			f->first = 0;
		}

		poolFree(&f->dataBuf, tmp->data);
		poolFree(&f->fifoElemBuf, tmp);
	}
}

void debugPrintFifo(Fifo* f)
{
	assert(f);

	fprintf(stderr, "fifo debug print\n");
	fprintf(stderr, "dataSize %u\n", f->dataSize);
	fprintf(stderr, "maxElems %u\n", f->maxElems);

	FifoElem* ptr = f->first;

	while(ptr)
	{
		fprintf(stderr, "fifo elem %p prev %p next %p data %p\n", ptr, ptr->prev, ptr->next, ptr->data);
		ptr = ptr->next;
	}
}

#if defined (__cplusplus)
}
#endif
