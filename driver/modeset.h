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

	drmModeModeInfo mode;
	uint32_t conn;
	uint32_t crtc;
	drmModeCrtc *saved_crtc;
	uint32_t width;
	uint32_t height;
	uint32_t handle;
} modeset_dev;

typedef struct modeset_display {
	char name[32];
	uint32_t mmWidth, mmHeight;
	uint32_t resWidth, resHeight;
	uint32_t connectorID;
} modeset_display;

typedef struct modeset_display_mode {
	uint32_t connectorID;
	uint32_t modeID;
	uint32_t resWidth, resHeight;
	uint32_t refreshRate;
} modeset_display_mode;

typedef struct modeset_display_surface {
	uint32_t connectorID;
	uint32_t modeID;
	uint32_t encoderID;
	uint32_t crtcID;
} modeset_display_surface;

modeset_dev* modeset_create(int fd);
void modeset_present_buffer(int fd, modeset_dev* dev, _image* buffer);
void modeset_destroy(int fd, modeset_dev* dev);
int modeset_create_fb(int fd, _image *buf);
void modeset_destroy_fb(int fd, _image *buf);
int modeset_fb_for_dev(int fd, modeset_dev* dev, _image* buffer);

void modeset_enum_displays(int fd, uint32_t* numDisplays, modeset_display* displays);
void modeset_enum_modes_for_display(int fd, uint32_t display, uint32_t* numModes, modeset_display_mode* modes);
void modeset_create_surface_for_mode(int fd, uint32_t display, uint32_t mode, modeset_display_surface* surface);

#if defined (__cplusplus)
}
#endif
