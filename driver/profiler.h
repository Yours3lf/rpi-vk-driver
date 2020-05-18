#if defined (__cplusplus)
extern "C" {
#endif

#define _POSIX_C_SOURCE 199309L

#include "map.h"

#include <time.h>

#define MILLION 1000000.0

#define MAX_FUNCTIONS 8192

typedef struct
{
	map funcDatabase;
	uint32_t frameCounter;
} profiler;

typedef struct
{
	char* funcName; //stores function name
	double timeSpent; //stores time spent in function in milliseconds
	struct timespec start; //for timekeeping
	uint32_t inProgress;
} funcData;

void initProfiler();
void startMeasure(void* func, const char* funcName);
void endMeasure(void* func);
void endFrame();
double getTimeSpent(void* func);
void profilePrintResults();

#if defined (__cplusplus)
}
#endif
