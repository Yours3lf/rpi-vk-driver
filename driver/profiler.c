#if defined (__cplusplus)
extern "C" {
#endif

#include "profiler.h"

#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include <semaphore.h>

static profiler* globalProfiler = 0;
static sem_t globalProfilerSem;

#define SKIPPED_FRAMES 100

void initProfiler()
{
	if(!globalProfiler)
	{
		sem_init(&globalProfilerSem, 0, 0);
		sem_post(&globalProfilerSem);

		globalProfiler = (profiler*)malloc(sizeof(profiler));
		globalProfiler->funcDatabase = createMap(malloc(sizeof(mapElem) * MAX_FUNCTIONS), MAX_FUNCTIONS);
		globalProfiler->frameCounter = 0;
	}
}

void startMeasure(void* func, const char* funcName)
{
	initProfiler();

	sem_wait(&globalProfilerSem);
	{
		assert(globalProfiler);
		assert(func);
		assert(funcName);
		funcData* data = getMapElement(globalProfiler->funcDatabase, func);
		if(!data)
		{
			data = malloc(sizeof(funcData));
			unsigned len = strlen(funcName)+1;
			data->funcName = malloc(len);
			memcpy(data->funcName, funcName, len);
			data->timeSpent = 0.0;
			data->inProgress = 0;
			data->start.tv_nsec = 0;
			data->start.tv_sec = 0;
			setMapElement(&globalProfiler->funcDatabase, func, data);
		}

		assert(!data->inProgress);
		data->inProgress = 1;
		clock_gettime(CLOCK_REALTIME, &data->start);
	}
	sem_post(&globalProfilerSem);
}

void endMeasure(void* func)
{
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);

	sem_wait(&globalProfilerSem);
	{
		assert(globalProfiler);
		assert(func);

		funcData* data = getMapElement(globalProfiler->funcDatabase, func);
		assert(data);
		assert(data->inProgress);
		data->inProgress = 0;

		if(globalProfiler->frameCounter > SKIPPED_FRAMES)
		{
			if((end.tv_nsec - data->start.tv_nsec) < 0)
			{
				data->timeSpent += (end.tv_sec - data->start.tv_sec - 1) * 0.001 + (1000000000 + end.tv_nsec - data->start.tv_nsec) / MILLION;
			}
			else
			{
				data->timeSpent += (end.tv_sec - data->start.tv_sec) * 0.001 + (end.tv_nsec - data->start.tv_nsec) / MILLION;
			}
		}
	}
	sem_post(&globalProfilerSem);
}

void endFrame()
{
	if(!globalProfiler) return;

	sem_wait(&globalProfilerSem);
	{
		globalProfiler->frameCounter++;
	}
	sem_post(&globalProfilerSem);
}

double getTimeSpent(void* func)
{
	assert(globalProfiler);
	assert(func);

	funcData* data = getMapElement(globalProfiler->funcDatabase, func);
	if(!data)
	{
		return 0;
	}

	return data->timeSpent / (double)(globalProfiler->frameCounter - SKIPPED_FRAMES);
}

void profilePrintResults()
{
	if(!globalProfiler) return;

	funcData profileResults[MAX_FUNCTIONS];
	memset(profileResults, 0, sizeof(profileResults));

	int32_t numFunctions = 0;

	//insertion sort, linear search
	for(uint32_t c = 0; c < globalProfiler->funcDatabase.maxData; ++c)
	{
		if(!globalProfiler->funcDatabase.elements[c].data)
		{
			continue;
		}

		funcData* data = globalProfiler->funcDatabase.elements[c].data;

		for(int32_t d = 0; d < MAX_FUNCTIONS; ++d)
		{
			if(!profileResults[d].funcName)
			{
				//empty slot, just insert here
				memcpy(&profileResults[d], data, sizeof(funcData));

				numFunctions++;
				break;
			}
			else
			{
				if(profileResults[d].timeSpent > data->timeSpent)
				{
					//found a function with more time spent, so we need to insert before it
					//need to insert before d

					//move all functions up one
					for(int32_t e = numFunctions - 1; e >= d; e--)
					{
						memcpy(&profileResults[e+1], &profileResults[e], sizeof(funcData));
					}

					//insert in place of the function with more time spent
					memcpy(&profileResults[d], data, sizeof(funcData));

					numFunctions++;
					break;
				}
			}
		}
	}

	//print most time spent first
	fprintf(stderr, "\nNum frames: %u\n", globalProfiler->frameCounter - SKIPPED_FRAMES);
	fprintf(stderr, "Num functions touched: %u\n", numFunctions);
	double overHead = 0.0;
	for(int32_t c = numFunctions - 1; c >= 0; --c)
	{
		overHead += profileResults[c].timeSpent;
	}
	//
	fprintf(stderr, "Total driver overhead: %lf ms\n", (overHead - profileResults[numFunctions - 1].timeSpent) / (double)(globalProfiler->frameCounter - SKIPPED_FRAMES));

	uint32_t counter = 0;
	for(int32_t c = numFunctions - 1; c >= 0; --c)
	{
		double timeSpent = profileResults[c].timeSpent / (double)(globalProfiler->frameCounter - SKIPPED_FRAMES);

		if(timeSpent < 0.0001) continue;

		fprintf(stderr, "#%u %-30s: %lf ms\n", ++counter, profileResults[c].funcName, timeSpent);
		if(counter >= 10)
		{
			//break;
		}
	}
}

#if defined (__cplusplus)
}
#endif

