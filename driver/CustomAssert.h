#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <signal.h>

enum { HAVE_TRAP_INSTRUCTION = 1, };
__attribute__((gnu_inline, always_inline))
__inline__ static void DEBUG_BREAK(void)
{
	__asm__ volatile(".inst 0xe7f001f0");
}

#ifdef DEBUG_BUILD
  #define assert(expr) \
    if( expr ){} \
    else \
    { \
	  printf("Assert failed: %s\n" \
			 "File: %s\n" \
			 "Line: %i\n", #expr, __FILE__, __LINE__); \
      DEBUG_BREAK(); \
    }
#else
  #define assert(expr) //do nothing
#endif

#if defined (__cplusplus)
}
#endif
