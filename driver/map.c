#include "map.h"

uint32_t getIndex(uint32_t key, uint32_t maxData)
{
	//multiplicative hash
	//with power of 2 W
	uint32_t a = 0x678DDE6F;
	return ((a * key) >> (32 - maxData)) % maxData;
}

void* getMapElement(map m, uint32_t key)
{
	assert(m.maxData > 0);
	assert(m.elements);
	uint32_t index = getIndex(key, m.maxData);
	//linear open addressing
	while(m.elements[index].key != key && m.elements[index].data != 0){index = (index + 1) % m.maxData;}
	return m.elements[index].data;
}

void setMapElement(map* m, uint32_t key, void* data)
{
	assert(m);
	assert(m->elements);
	assert(m->maxData > 0);
	uint32_t index = getIndex(key, m->maxData);
	while(m->elements[index].key != key && m->elements[index].data != 0){index = (index + 1) % m->maxData;}
	m->elements[index].data = data;
	m->elements[index].key = key;
}

void deleteMapElement(map* m, uint32_t key)
{
	assert(m);
	assert(m->elements);
	assert(m->maxData > 0);
	uint32_t index = getIndex(key, m->maxData);
	while(m->elements[index].key != key){++index;}
	m->elements[index].data = 0;
}

map createMap(void* buf, uint32_t maxData)
{
	map m =
	{
		.elements = buf,
		.maxData = maxData
	};

	//lazy hashing
	//0 means bucket is empty
	for(uint32_t c = 0; c < m.maxData; ++c)
	{
		m.elements[c].data = 0;
	}

	return m;
}

void destroyMap(map* m)
{
	//actual memory freeing is done by caller
	m->elements = 0;
	m->maxData = 0;
}