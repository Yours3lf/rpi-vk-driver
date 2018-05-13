#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include "CustomAssert.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <drm/vc4_drm.h>

#define DRM_IOCTL_CTRL_DEV_FILE_NAME "/dev/dri/card0"
#define DRM_IOCTL_RENDER_DEV_FILE_NAME "/dev/dri/renderD128" //TODO does this need to be dynamic? (eg. iterate through renderDn?)

extern int controlFd;
extern int renderFd;

int openIoctl();
void closeIoctl();

int vc4_get_chip_info(int fd);
int vc4_has_feature(int fd, uint32_t feature);
int vc4_test_tiling(int fd);
uint64_t vc4_bo_get_tiling(int fd, uint32_t bo, uint64_t mod);
int vc4_bo_set_tiling(int fd, uint32_t bo, uint64_t mod);
void* vc4_bo_map_unsynchronized(int fd, uint32_t bo, uint32_t size);
int vc4_bo_wait_ioctl(int fd, uint32_t handle, uint64_t timeout_ns);
int vc4_seqno_wait_ioctl(int fd, uint64_t seqno, uint64_t timeout_ns);
int vc4_bo_flink(int fd, uint32_t bo, uint32_t *name);
uint32_t vc4_bo_alloc_shader(int fd, const void *data, uint32_t* size);
uint32_t vc4_bo_open_name(int fd, uint32_t name);
uint32_t vc4_bo_alloc(int fd, uint32_t size, const char *name);
void vc4_bo_free(int fd, uint32_t bo, void* mappedAddr, uint32_t size);
int vc4_bo_unpurgeable(int fd, uint32_t bo, int hasMadvise);
void vc4_bo_purgeable(int fd, uint32_t bo, int hasMadvise);
void vc4_bo_label(int fd, uint32_t bo, const char* name);

#if defined (__cplusplus)
}
#endif
