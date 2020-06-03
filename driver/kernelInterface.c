#define _GNU_SOURCE
#include "kernelInterface.h"
#include <stdatomic.h>

atomic_int refCounter = 0;
int controlFd = 0;
//int renderFd = 0;

int openIoctl()
{
	if(!controlFd)
	{
		controlFd = open(DRM_IOCTL_CTRL_DEV_FILE_NAME, O_RDWR | O_CLOEXEC);
		if (controlFd < 0) {
			fprintf(stderr, "Can't open device file: %s \nError: %s\n", DRM_IOCTL_CTRL_DEV_FILE_NAME, strerror(errno));
			return -1;
		}
	}

	/*if(!renderFd)
	{
		renderFd = open(DRM_IOCTL_RENDER_DEV_FILE_NAME, O_RDWR | O_CLOEXEC);
		if (renderFd < 0) {
			printf("Can't open device file: %s \nError: %s\n", DRM_IOCTL_RENDER_DEV_FILE_NAME, strerror(errno));
			return -1;
		}
	}*/

	++refCounter;

	return 0;
}

void closeIoctl(int fd)
{
	if (--refCounter == 0)
	{
		close(fd);
	}
}

static uint32_t align(uint32_t num, uint32_t alignment)
{
	uint32_t mod = num%alignment;
	if(!mod)
	{
		return num;
	}
	else
	{
		return num + alignment - mod;
	}
}

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
					  uint32_t* vriMemorySize)
{
	assert(fd);
	assert(technologyVersion);
	assert(IDstrUINT);
	assert(vpmMemorySize);
	assert(hdrSupported);
	assert(numSemaphores);
	assert(numTMUperSlice);
	assert(numQPUperSlice);
	assert(numSlices);
	assert(v3dRevision);
	assert(tileBufferDoubleBufferModeSupported);
	assert(tileBufferSize);
	assert(vriMemorySize);

	struct drm_vc4_get_param ident0 = {
		.param = DRM_VC4_PARAM_V3D_IDENT0,
	};
	struct drm_vc4_get_param ident1 = {
		.param = DRM_VC4_PARAM_V3D_IDENT1,
	};
	struct drm_vc4_get_param ident2 = {
		.param = DRM_VC4_PARAM_V3D_IDENT2,
	};
	int ret;

	ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_PARAM, &ident0);
	if (ret != 0) {
		if (errno == EINVAL) {
			/* Backwards compatibility with 2835 kernels which
						 * only do V3D 2.1.
						 */
			fprintf(stderr, "Couldn't get V3D IDENT0: %s\n",
				   strerror(errno));
			return 0; //21
		} else {
			fprintf(stderr, "Couldn't get V3D IDENT0: %s\n",
				   strerror(errno));
			return 0;
		}
	}
	ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_PARAM, &ident1);
	if (ret != 0) {
		fprintf(stderr, "Couldn't get V3D IDENT1: %s\n",
			   strerror(errno));
		return 0;
	}
	ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_PARAM, &ident2);
	if (ret != 0) {
		fprintf(stderr, "Couldn't get V3D IDENT2: %s\n",
			   strerror(errno));
		return 0;
	}

	*technologyVersion = (ident0.value >> 24) & 0xff;
	*IDstrUINT = (ident0.value >> 0) & 0x00ffffff;

	*vpmMemorySize = ((ident1.value >> 28) & 0xf) * 1024; //multiples of 1K
	*hdrSupported = (ident1.value >> 24) & 0xf;
	*numSemaphores = (ident1.value >> 16) & 0xff;
	*numTMUperSlice = (ident1.value >> 12) & 0xf;
	*numQPUperSlice = (ident1.value >> 8) & 0xf;
	*numSlices = (ident1.value >> 4) & 0xf;
	*v3dRevision = (ident1.value >> 0) & 0xf;

	*tileBufferDoubleBufferModeSupported = (ident2.value >> 8) & 0xf;
	*tileBufferSize = (ident2.value >> 4) & 0xf;
	*vriMemorySize = (ident2.value >> 0) & 0xf;

	uint32_t v3d_ver = (*technologyVersion) * 10 + (*v3dRevision);

	if(v3d_ver != 21 && v3d_ver != 26)
	{
		printf("v3d_ver unsupported: %u\n", v3d_ver);
		return 0;
	}

	return 1;
}

int vc4_has_feature(int fd, uint32_t feature)
{
	assert(fd);

	struct drm_vc4_get_param p = {
		.param = feature,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_PARAM, &p);

	if (ret != 0)
	{
		fprintf(stderr, "Couldn't determine if VC4 has feature: %s\n", strerror(errno));
		return 0;
	}

	return p.value;
}

int vc4_test_tiling(int fd)
{
	assert(fd);

	/* Test if the kernel has GET_TILING; it will return -EINVAL if the
	 * ioctl does not exist, but -ENOENT if we pass an impossible handle.
	 * 0 cannot be a valid GEM object, so use that.
	 */
	struct drm_vc4_get_tiling get_tiling = {
		.handle = 0x0,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_TILING, &get_tiling);
	if (ret == -1 && errno == ENOENT)
	{
		return 1;
	}

	return 0;
}

uint64_t vc4_bo_get_tiling(int fd, uint32_t bo, uint64_t mod)
{
	assert(fd);
	assert(bo);

	struct drm_vc4_get_tiling get_tiling = {
		.handle = bo,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_VC4_GET_TILING, &get_tiling);

	if (ret != 0) {
		return DRM_FORMAT_MOD_LINEAR; //0
	} else if (mod == DRM_FORMAT_MOD_INVALID) {
		return get_tiling.modifier;
	} else if (mod != get_tiling.modifier) {
		fprintf(stderr, "Modifier 0x%llx vs. tiling (0x%llx) mismatch\n",
			   (long long)mod, get_tiling.modifier);
		return -1;
	}

	return -1;
}

int vc4_bo_set_tiling(int fd, uint32_t bo, uint64_t mod)
{
	assert(fd);
	assert(bo);

	struct drm_vc4_set_tiling set_tiling = {
		.handle = bo,
				.modifier = mod,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_VC4_SET_TILING,
					   &set_tiling);
	if (ret != 0)
	{
		fprintf(stderr, "Couldn't set tiling: %s, bo %u, mod %llu\n",
			   strerror(errno), bo, mod);
		return 0;
	}

	return 1;
}

uint32_t vc4_set_madvise(int fd, uint32_t bo, uint32_t needed, int hasMadvise)
{
	assert(fd);
	assert(bo);

	//VC4_MADV_WILLNEED			0
	//VC4_MADV_DONTNEED			1
	struct drm_vc4_gem_madvise arg = {
		.handle = bo,
		.madv = !needed,
	};

	if (!hasMadvise)
		return 1;

	if (drmIoctl(fd, DRM_IOCTL_VC4_GEM_MADVISE, &arg))
	{
		fprintf(stderr, "BO madvise failed: %s, bo %u, needed %u\n",
			   strerror(errno), bo, needed);
		return 0;
	}

	return arg.retained;
}

void* vc4_bo_map_unsynchronized(int fd, uint32_t bo, uint32_t offset, uint32_t size)
{
	assert(fd);
	assert(bo);
	assert(size);

	int ret;

	struct drm_vc4_mmap_bo map;
	memset(&map, 0, sizeof(map));
	map.handle = bo;
	ret = drmIoctl(fd, DRM_IOCTL_VC4_MMAP_BO, &map);
	if (ret != 0) {
		fprintf(stderr, "Couldn't map unsync: %s, bo %u, offset %u, size %u\n", strerror(errno), bo, offset, size);
		return 0;
	}

	void* mapPtr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
						fd, map.offset + offset);
	if (mapPtr == MAP_FAILED) {
		fprintf(stderr, "mmap of bo %d (offset 0x%016llx, size %d) failed\n",
			   bo, (long long)map.offset + offset, size);
		return 0;
	}

	return mapPtr;
}

void vc4_bo_unmap_unsynchronized(int fd, void* ptr, uint32_t size)
{
	assert(fd);
	assert(ptr);
	assert(size);

	munmap(ptr, size);
}

int vc4_bo_wait(int fd, uint32_t bo, uint64_t timeout_ns)
{
	assert(fd);
	assert(bo);

	struct drm_vc4_wait_bo wait = {
		.handle = bo,
				.timeout_ns = timeout_ns,
	};

	//printf("Wait for BO: %u\n", bo);

	int ret = drmIoctl(fd, DRM_IOCTL_VC4_WAIT_BO, &wait);
	if (ret) {
		if (ret != -ETIME) {
			fprintf(stderr, "BO wait failed: %s, bo %u, timeout %llu\n",
				   strerror(errno), bo, timeout_ns);
		}

		return 0;
	}

	return 1;
}

int vc4_seqno_wait(int fd, uint64_t* lastFinishedSeqno, uint64_t seqno, uint64_t* timeout_ns)
{
	assert(fd);
	assert(lastFinishedSeqno);
	assert(timeout_ns);

	if(!seqno)
		return 1;

	if (*lastFinishedSeqno >= seqno)
		return 1;

	struct drm_vc4_wait_seqno wait = {
		.seqno = seqno,
				.timeout_ns = *timeout_ns,
	};

	//printf("Wait for seqno: %llu\n", seqno);

	int ret = drmIoctl(fd, DRM_IOCTL_VC4_WAIT_SEQNO, &wait);
	if (ret) {
		if (errno != ETIME) {
			fprintf(stderr, "Seqno wait failed: %s, seqno %llu, timeout %llu\n",
				   strerror(errno), seqno, *timeout_ns);
			vc4_print_hang_state(controlFd);
		}
		else
		{
			//Timeout happened
			*timeout_ns = -1;
			return -1;
		}

		return 0;
	}

	*timeout_ns = wait.timeout_ns;
	*lastFinishedSeqno = seqno;
	return 1;
}

int vc4_bo_flink(int fd, uint32_t bo, uint32_t *name)
{
	assert(fd);
	assert(bo);
	assert(name);

	struct drm_gem_flink flink = {
		.handle = bo,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_GEM_FLINK, &flink);
	if (ret) {
		fprintf(stderr, "Failed to flink bo %d: %s\n",
			   bo, strerror(errno));
		return 0;
	}

	//bo->private = false;
	*name = flink.name;

	return 1;
}

uint32_t getBOAlignedSize(uint32_t size, uint32_t alignment)
{
	return align(size, alignment);
}

uint32_t vc4_bo_alloc_shader(int fd, const void *data, uint32_t* size)
{
	assert(fd);
	assert(data);
	assert(size);

	int ret;

	//kernel only requires alignmnet to sizeof(uint64_t), not an entire page
	uint32_t alignedSize = getBOAlignedSize(*size, sizeof(uint64_t));

	struct drm_vc4_create_shader_bo create = {
		.size = alignedSize,
				.data = (uintptr_t)data,
	};

	ret = drmIoctl(fd, DRM_IOCTL_VC4_CREATE_SHADER_BO,
				   &create);

	if (ret != 0) {
		fprintf(stderr, "Couldn't create shader: %s, size %u\n",
			   strerror(errno), *size);
		return 0;
	}

	*size = alignedSize;

	return create.handle;
}

uint32_t vc4_bo_open_name(int fd, uint32_t name)
{
	assert(fd);
	assert(name);

	struct drm_gem_open o = {
		.name = name
	};
	int ret = drmIoctl(fd, DRM_IOCTL_GEM_OPEN, &o);
	if (ret) {
		fprintf(stderr, "Failed to open bo %d: %s\n",
			   name, strerror(errno));
		return 0;
	}

	return o.handle;
}

uint32_t vc4_bo_alloc(int fd, uint32_t size, const char *name)
{
	assert(fd);
	assert(size);

	struct drm_vc4_create_bo create;
	int ret;

	memset(&create, 0, sizeof(create));
	create.size = size;

	ret = drmIoctl(fd, DRM_IOCTL_VC4_CREATE_BO, &create);
	uint32_t handle = create.handle;

	if (ret != 0) {
		fprintf(stderr, "Couldn't alloc BO: %s, size %u\n",
			   strerror(errno), size);
		return 0;
	}

	vc4_bo_label(fd, handle, name);

#ifdef DEBUG_BUILD
	void* ptr = vc4_bo_map(fd, handle, 0, size);
	memset(ptr, 0, size);
#endif

	return handle;
}

void vc4_bo_free(int fd, uint32_t bo, void* mappedAddr, uint32_t size)
{
	assert(fd);
	assert(bo);
	assert(size);

	if (mappedAddr) {
		vc4_bo_unmap_unsynchronized(fd, mappedAddr, size);
	}

	struct drm_gem_close c;
	memset(&c, 0, sizeof(c));
	c.handle = bo;
	int ret = drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &c);
	if (ret != 0)
	{
		fprintf(stderr, "couldn't close object %d: %s\n", bo, strerror(errno));
	}
}

void vc4_bo_label(int fd, uint32_t bo, const char* name)
{
#ifdef DEBUG_BUILD
	assert(fd);
	assert(bo);

	char* str = name;
	if(!str) str = "";

	struct drm_vc4_label_bo label = {
		.handle = bo,
				.len = strlen(str),
				.name = (uintptr_t)str,
	};
	int ret = drmIoctl(fd, DRM_IOCTL_VC4_LABEL_BO, &label);
	if(ret)
	{
		fprintf(stderr, "BO label failed: %s, bo %u\n",
			   strerror(errno), bo);
	}
#endif
}

int vc4_bo_get_dmabuf(int fd, uint32_t bo)
{
	assert(fd);
	assert(bo);

	int boFd;
	int ret = drmPrimeHandleToFD(fd, bo,
								 O_CLOEXEC, &boFd);
	if (ret != 0) {
		fprintf(stderr, "Failed to export gem bo %d to dmabuf: %s\n",
			   bo, strerror(errno));
		return 0;
	}

	return boFd;
}

void* vc4_bo_map(int fd, uint32_t bo, uint32_t offset, uint32_t size)
{
	assert(fd);
	assert(bo);
	assert(size);

	void* map = vc4_bo_map_unsynchronized(fd, bo, offset, size);

	//wait infinitely
	int ok = vc4_bo_wait(fd, bo, WAIT_TIMEOUT_INFINITE);
	if (!ok) {
		fprintf(stderr, "BO wait for map failed: %s, bo %u, offset %u, size %u\n", strerror(errno), bo, offset, size);
		return 0;
	}

	return map;
}

void vc4_cl_submit(int fd, struct drm_vc4_submit_cl* submit, uint64_t* lastEmittedSeqno, uint64_t* lastFinishedSeqno)
{
	assert(fd);
	assert(submit);
	assert(lastEmittedSeqno);
	assert(lastFinishedSeqno);

	int ret = drmIoctl(fd, DRM_IOCTL_VC4_SUBMIT_CL, submit);

	static int warned = 0;
	if (ret && !warned) {
		fprintf(stderr, "Draw call returned %s.  "
			   "Expect corruption.\n", strerror(errno));
		warned = 1;
		assert(0);
	} else if (!ret) {
		*lastEmittedSeqno = submit->seqno;
	}

	if (*lastEmittedSeqno - *lastFinishedSeqno > 5) {
		uint64_t timeout = WAIT_TIMEOUT_INFINITE;
		//uint64_t timeout = 1000ull * 1000ull * 1000ull; //TODO waits too long...
		if (!vc4_seqno_wait(fd,
							lastFinishedSeqno,
							*lastFinishedSeqno > 0 ? *lastEmittedSeqno - 5 : *lastEmittedSeqno,
							&timeout))
		{
			fprintf(stderr, "Job throttling failed\n");
		}
	}
}

uint32_t vc4_create_perfmon(int fd, uint32_t* counters, uint32_t num_counters)
{
	assert(fd);
	assert(counters);
	assert(num_counters > 0);
	assert(num_counters <= DRM_VC4_MAX_PERF_COUNTERS);

	struct drm_vc4_perfmon_create arg =
	{
		.id = 0,
		.ncounters = num_counters,
	};

	for(uint32_t c = 0; c < num_counters; ++c)
	{
		arg.events[c] = counters[c];
	}

	if (drmIoctl(fd, DRM_IOCTL_VC4_PERFMON_CREATE, &arg))
	{
		fprintf(stderr, "Perfmon create failed: %s\n",
			   strerror(errno));
		return 0;
	}

	if(!arg.id)
	{
		fprintf(stderr, "Perfmon create failed (invalid ID): %s\n",
			   strerror(errno));
		return 0;
	}

	return arg.id;
}

void vc4_destroy_perfmon(int fd, uint32_t id)
{
	assert(fd);
	assert(id);

	struct drm_vc4_perfmon_destroy arg =
	{
		.id = id
	};

	if (drmIoctl(fd, DRM_IOCTL_VC4_PERFMON_DESTROY, &arg))
	{
		fprintf(stderr, "Perfmon destroy failed: %s\n",
			   strerror(errno));
	}
}

/*
 * Returns the values of the performance counters tracked by this
 * perfmon (as an array of ncounters * u64 values).
 *
 * No implicit synchronization is performed, so the user has to
 * guarantee that any jobs using this perfmon have already been
 * completed  (probably by blocking on the seqno returned by the
 * last exec that used the perfmon).
 */
void vc4_perfmon_get_values(int fd, uint32_t id, void* ptr)
{
	assert(fd);
	assert(id);
	assert(ptr);

	struct drm_vc4_perfmon_get_values arg =
	{
		.id = id,
		.values_ptr = ptr
	};

	if (drmIoctl(fd, DRM_IOCTL_VC4_PERFMON_GET_VALUES, &arg))
	{
		fprintf(stderr, "Perfmon get values failed: %s\n",
			   strerror(errno));
	}
}

void vc4_print_hang_state(int fd)
{
	assert(fd);

	struct drm_vc4_get_hang_state_bo bo_states[128];

	struct drm_vc4_get_hang_state arg =
	{
		/** Pointer to array of struct drm_vc4_get_hang_state_bo. */
		.bo = bo_states,
		/**
		 * On input, the size of the bo array.  Output is the number
		 * of bos to be returned.
		 */
		.bo_count = 128
	};

	if (drmIoctl(fd, DRM_IOCTL_VC4_GET_HANG_STATE, &arg))
	{
		fprintf(stderr, "vc4 get hang state failed: %s\n",
			   strerror(errno));
	}
	else
	{
		fprintf(stderr, "--------------\n");
		fprintf(stderr, "--------------\n");
		fprintf(stderr, "GPU hang state\n");
		for(uint32_t c = 0; c < arg.bo_count; ++c)
		{
			struct drm_vc4_get_hang_state_bo* bos = arg.bo;
			fprintf(stderr, "BO: %u, Addr: %u, Size: %u\n", bos[c].handle, bos[c].paddr, bos[c].size);
		}

		fprintf(stderr, "Start bin: %u, Start render: %u\n", arg.start_bin, arg.start_render);
		fprintf(stderr, "ct0ca: %u, ct0ea: %u\n", arg.ct0ca, arg.ct0ea);
		fprintf(stderr, "ct1ca: %u, ct1ea: %u\n", arg.ct1ca, arg.ct1ea);
		fprintf(stderr, "ct0cs: %u, ct1cs: %u\n", arg.ct0cs, arg.ct1cs);
		fprintf(stderr, "ct0ra0: %u, ct1ra0: %u\n", arg.ct0ra0, arg.ct1ra0);
		fprintf(stderr, "bpca: %u, bpcs: %u\n", arg.bpca, arg.bpcs);
		fprintf(stderr, "bpoa: %u, bpos: %u\n", arg.bpoa, arg.bpos);
		fprintf(stderr, "vpmbase: %u: %u\n", arg.vpmbase);
		fprintf(stderr, "dbge: %u: %u\n", arg.dbge);
		fprintf(stderr, "fdbgo: %u: %u\n", arg.fdbgo);
		fprintf(stderr, "fdbgb: %u: %u\n", arg.fdbgb);
		fprintf(stderr, "fdbgr: %u: %u\n", arg.fdbgr);
		fprintf(stderr, "fdbgs: %u: %u\n", arg.fdbgs);
		fprintf(stderr, "errstat: %u: %u\n", arg.errstat);
	}
}
