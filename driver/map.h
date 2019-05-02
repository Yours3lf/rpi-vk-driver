#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "CustomAssert.h"

typedef struct
{
	void* data;
	uint32_t key;
} mapElem;

typedef struct
{
	mapElem* elements;
	uint32_t maxData;
} map;

void* getMapElement(map m, uint32_t key);
void setMapElement(map* m, uint32_t key, void* data);
void deleteMapElement(map* m, uint32_t key);
map createMap(void* buf, uint32_t maxData);
void destroyMap(map* m);


#if defined (__cplusplus)
}
#endif



