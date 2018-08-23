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
#include "CustomAssert.h"

typedef struct VkImage_T
{
	uint32_t handle;
	uint32_t fb; //needed for swapchain
	uint32_t width, height, depth;
	uint32_t paddedWidth, paddedHeight;
	uint32_t miplevels, layers, samples, size;
	uint32_t stride; //the number of bytes from one row of pixels in memory to the next row of pixels in memory (aka pitch)
	uint32_t usageBits;
	uint32_t format;
} _image;

typedef struct modeset_dev {
	struct modeset_dev *next;

	//unsigned int front_buf;
	//struct modeset_buf bufs[2];

	drmModeModeInfo mode;
	uint32_t conn;
	uint32_t crtc;
	drmModeCrtc *saved_crtc;
	uint32_t width;
	uint32_t height;
	uint32_t handle;
} modeset_dev;

modeset_dev* modeset_create(int fd);
void modeset_present_buffer(int fd, modeset_dev* dev, _image* buffer);
void modeset_destroy(int fd, modeset_dev* dev);
int modeset_create_fb(int fd, _image *buf);
void modeset_destroy_fb(int fd, _image *buf);
int modeset_fb_for_dev(int fd, modeset_dev* dev, _image* buffer);

#if defined (__cplusplus)
}
#endif
