#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include "driver/CustomAssert.h"
#include "driver/fifo.h"

int main() {

	uint32_t maxElems = 5;
	void* dataBuf = malloc(sizeof(uint32_t) * maxElems);
	void* elemBuf = malloc(sizeof(FifoElem) * maxElems);
	Fifo f = createFifo(dataBuf, elemBuf, maxElems, sizeof(uint32_t));

	debugPrintFifo(&f);

	for(uint32_t data = 1; data <= 5; ++data)
	{
		fifoAdd(&f, &data);
		debugPrintFifo(&f);
	}

	for(uint32_t c = 0; c < 5; ++c)
	{
		uint32_t data = 0;
		fifoRemove(&f, &data);
		debugPrintFifo(&f);
		fprintf(stderr, "data %u\n", data);
	}

	destroyFifo(&f);

	return 0;
}
