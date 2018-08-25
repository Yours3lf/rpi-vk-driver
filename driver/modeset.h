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
	uint32_t miplevels, samples;
	uint32_t layers; //number of views for multiview/stereo
	uint32_t size; //overall size including padding
	uint32_t stride; //the number of bytes from one row of pixels in memory to the next row of pixels in memory (aka pitch)
	uint32_t usageBits;
	uint32_t format;
	uint32_t imageSpace;
	uint32_t tiling;
	uint32_t needToClear;
	uint32_t clearColor[2];
	uint32_t layout;
	uint32_t concurrentAccess; //TODO
	uint32_t numQueueFamiliesWithAccess;
	uint32_t* queueFamiliesWithAccess;
	uint32_t preTransformMode;
	uint32_t compositeAlpha;
	uint32_t presentMode;
	uint32_t clipped;
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
