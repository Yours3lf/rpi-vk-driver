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

#include "common.h"

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
