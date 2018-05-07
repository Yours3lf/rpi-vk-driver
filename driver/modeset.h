#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

typedef struct modeset_dev modeset_dev;

int modeset_open(const char* node);
modeset_dev* modeset_create();
void modeset_swapbuffer(modeset_dev* dev);
void modeset_destroy(modeset_dev* dev);
void modeset_close();

#if defined (__cplusplus)
}
#endif
