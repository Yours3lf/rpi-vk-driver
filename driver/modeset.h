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

typedef struct modeset_plane {
	drmModePlanePtr plane;
	uint32_t currentConnectorID;
	uint32_t possibleConnectors[16];
	uint32_t numPossibleConnectors;
} modeset_plane;

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
	drmModeConnectorPtr connector;
	drmModeCrtcPtr crtc;
	uint32_t modeID;
	uint32_t savedState;
} modeset_display_surface;

typedef struct modeset_saved_state {
	uint32_t used;
	drmModeConnectorPtr conn;
	drmModeCrtcPtr crtc;
} modeset_saved_state;

modeset_saved_state modeset_saved_states[32];

void modeset_enum_displays(int fd, uint32_t* numDisplays, modeset_display* displays);
void modeset_enum_modes_for_display(int fd, uint32_t display, uint32_t* numModes, modeset_display_mode* modes);
void modeset_create_surface_for_mode(int fd, uint32_t display, uint32_t mode, modeset_display_surface* surface);
void modeset_create_fb_for_surface(int fd, _image* buf, modeset_display_surface* surface);
void modeset_destroy_fb(int fd, _image* buf);
void modeset_present(int fd, _image* buf, modeset_display_surface* surface);
void modeset_acquire_image(int fd, _image** buf, modeset_display_surface** surface);
void modeset_destroy_surface(int fd, modeset_display_surface* surface);
void modeset_debug_print(int fd);

#if defined (__cplusplus)
}
#endif
