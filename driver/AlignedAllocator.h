#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

void* alignedAlloc( unsigned bytes, unsigned alignment );
void alignedFree( void* p );

#if defined (__cplusplus)
}
#endif

