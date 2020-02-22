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
#include <xf86drm.h>
#include <xf86drmMode.h>

//normal nodes?
//modesetting can be done
#define DRM_IOCTL_CTRL_DEV_FILE_NAME "/dev/dri/card0"

//render nodes
//no modesetting --> so only for offscreen rendering
#define DRM_IOCTL_RENDER_DEV_FILE_NAME "/dev/dri/renderD128"

#define WAIT_TIMEOUT_INFINITE 0xffffffffffffffffull
#define ARM_PAGE_SIZE 4096

extern int controlFd;
//extern int renderFd;

int openIoctl();
void closeIoctl();

int vc4_get_chip_info(int fd,
					  uint32_t* technologyVersion,
					  uint32_t* IDstrUINT,
					  uint32_t* vpmMemorySize,
					  uint32_t* hdrSupported,
					  uint32_t* numSemaphores,
					  uint32_t* numTMUperSlice,
					  uint32_t* numQPUperSlice,
					  uint32_t* numSlices,
					  uint32_t* v3dRevision,
					  uint32_t* tileBufferDoubleBufferModeSupported,
					  uint32_t* tileBufferSize,
					  uint32_t* vriMemorySize);
int vc4_has_feature(int fd, uint32_t feature);
int vc4_test_tiling(int fd);
uint64_t vc4_bo_get_tiling(int fd, uint32_t bo, uint64_t mod);
int vc4_bo_set_tiling(int fd, uint32_t bo, uint64_t mod);
void* vc4_bo_map_unsynchronized(int fd, uint32_t bo, uint32_t offset, uint32_t size);
void vc4_bo_unmap_unsynchronized(int fd, void* ptr, uint32_t size);
//int vc4_bo_wait_ioctl(int fd, uint32_t handle, uint64_t timeout_ns);
int vc4_bo_wait(int fd, uint32_t bo, uint64_t timeout_ns);
//int vc4_seqno_wait_ioctl(int fd, uint64_t seqno, uint64_t timeout_ns);
int vc4_seqno_wait(int fd, uint64_t* lastFinishedSeqno, uint64_t seqno, uint64_t* timeout_ns);
int vc4_bo_flink(int fd, uint32_t bo, uint32_t *name);
uint32_t vc4_bo_alloc_shader(int fd, const void *data, uint32_t* size);
uint32_t vc4_bo_open_name(int fd, uint32_t name);
uint32_t vc4_bo_alloc(int fd, uint32_t size, const char *name);
void vc4_bo_free(int fd, uint32_t bo, void* mappedAddr, uint32_t size);
uint32_t vc4_set_madvise(int fd, uint32_t bo, uint32_t needed, int hasMadvise);
uint32_t vc4_create_perfmon(int fd, uint32_t* counters, uint32_t num_counters);
void vc4_destroy_perfmon(int fd, uint32_t id);
void vc4_perfmon_get_values(int fd, uint32_t id, void* ptr);
void vc4_bo_label(int fd, uint32_t bo, const char* name);
int vc4_bo_get_dmabuf(int fd, uint32_t bo);
void* vc4_bo_map(int fd, uint32_t bo, uint32_t offset, uint32_t size);
void vc4_cl_submit(int fd, struct drm_vc4_submit_cl* submit, uint64_t* lastEmittedSeqno, uint64_t* lastFinishedSeqno);
uint32_t getBOAlignedSize(uint32_t size, uint32_t alignment);
void vc4_print_hang_state(int fd);

//TODO perfmon

#if defined (__cplusplus)
}
#endif
