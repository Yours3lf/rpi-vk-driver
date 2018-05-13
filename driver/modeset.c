/*
 * modeset - DRM Double-Buffered Modesetting Example
 *
 * Written 2012 by David Herrmann <dh.herrmann@googlemail.com>
 * Dedicated to the Public Domain.
 */

/*
 * DRM Modesetting Howto
 * This document describes the DRM modesetting API. Before we can use the DRM
 * API, we have to include xf86drm.h and xf86drmMode.h. Both are provided by
 * libdrm which every major distribution ships by default. It has no other
 * dependencies and is pretty small.
 *
 * Please ignore all forward-declarations of functions which are used later. I
 * reordered the functions so you can read this document from top to bottom. If
 * you reimplement it, you would probably reorder the functions to avoid all the
 * nasty forward declarations.
 *
 * For easier reading, we ignore all memory-allocation errors of malloc() and
 * friends here. However, we try to correctly handle all other kinds of errors
 * that may occur.
 *
 * All functions and global variables are prefixed with "modeset_*" in this
 * file. So it should be clear whether a function is a local helper or if it is
 * provided by some external library.
 */

#include "modeset.h"

/*
 * Previously, we used the modeset_dev objects to hold buffer informations, too.
 * Technically, we could have split them but avoided this to make the
 * example simpler.
 * However, in this example we need 2 buffers. One back buffer and one front
 * buffer. So we introduce a new structure modeset_buf which contains everything
 * related to a single buffer. Each device now gets an array of two of these
 * buffers.
 * Each buffer consists of width, height, stride, size, handle, map and fb-id.
 * They have the same meaning as before.
 *
 * Each device also gets a new integer field: front_buf. This field contains the
 * index of the buffer that is currently used as front buffer / scanout buffer.
 * In our example it can be 0 or 1. We flip it by using XOR:
 *   dev->front_buf ^= dev->front_buf
 *
 * Everything else stays the same.
 */

//static struct modeset_dev *modeset_list = NULL;

static int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
				 struct modeset_dev *dev);
static int modeset_create_fb(int fd, struct modeset_buf *buf);
static void modeset_destroy_fb(int fd, struct modeset_buf *buf);
static int modeset_setup_dev(int fd, drmModeRes *res, drmModeConnector *conn,
				 struct modeset_dev *dev);


/*
 * When the linux kernel detects a graphics-card on your machine, it loads the
 * correct device driver (located in kernel-tree at ./drivers/gpu/drm/<xy>) and
 * provides two character-devices to control it. Udev (or whatever hotplugging
 * application you use) will create them as:
 *     /dev/dri/card0
 *     /dev/dri/controlID64
 * We only need the first one. You can hard-code this path into your application
 * like we do here, but it is recommended to use libudev with real hotplugging
 * and multi-seat support. However, this is beyond the scope of this document.
 * Also note that if you have multiple graphics-cards, there may also be
 * /dev/dri/card1, /dev/dri/card2, ...
 *
 * We simply use /dev/dri/card0 here but the user can specify another path on
 * the command line.
 *
 * modeset_open(out, node): This small helper function opens the DRM device
 * which is given as @node. The new fd is stored in @out on success. On failure,
 * a negative error code is returned.
 * After opening the file, we also check for the DRM_CAP_DUMB_BUFFER capability.
 * If the driver supports this capability, we can create simple memory-mapped
 * buffers without any driver-dependent code. As we want to avoid any radeon,
 * nvidia, intel, etc. specific code, we depend on DUMB_BUFFERs here.
 */

modeset_dev* modeset_create(int fd)
{
	modeset_dev* ret_dev = 0;
	drmModeRes *res;
	drmModeConnector *conn;
	struct modeset_dev *dev;
	int ret;

	uint64_t has_dumb;
	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
		printf("drm device does not support dumb buffers\n");
		return 0;
	}

	// retrieve resources
	res = drmModeGetResources(fd);
	if (!res) {
		printf("cannot retrieve DRM resources (%d): %m\n", errno);
		return 0;
	}

	// iterate all connectors
	for (unsigned i = 0; i < res->count_connectors; ++i) {
		// get information for each connector
		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn) {
			printf("cannot retrieve DRM connector %u:%u (%d): %m\n", i, res->connectors[i], errno);
			continue;
		}

		// create a device structure
		dev = malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));
		dev->conn = conn->connector_id;

		// call helper function to prepare this connector
		ret = modeset_setup_dev(fd, res, conn, dev);
		if (ret) {
			if (ret != -ENOENT) {
				errno = -ret;
				printf("cannot setup device for connector %u:%u (%d): %m\n", i, res->connectors[i], errno);
			}
			free(dev);
			drmModeFreeConnector(conn);
			continue;
		}

		// free connector data and link device into global list
		drmModeFreeConnector(conn);
		dev->next = ret_dev;
		ret_dev = dev;
	}

	// free resources again
	drmModeFreeResources(res);

	struct modeset_dev *iter;
	struct modeset_buf *buf;
	for (iter = ret_dev; iter; iter = iter->next) {
		iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
		buf = &iter->bufs[iter->front_buf];
		ret = drmModeSetCrtc(fd, iter->crtc, buf->fb, 0, 0,
					 &iter->conn, 1, &iter->mode);
		if (ret)
			printf("cannot set CRTC for connector %u (%d): %m\n",
				iter->conn, errno);
	}

	return ret_dev;
}

/*
 * modeset_setup_dev() sets up all resources for a single device. It mostly
 * stays the same, but one thing changes: We allocate two framebuffers instead
 * of one. That is, we call modeset_create_fb() twice.
 * We also copy the width/height information into both framebuffers so
 * modeset_create_fb() can use them without requiring a pointer to modeset_dev.
 */

static int modeset_setup_dev(int fd, drmModeRes *res, drmModeConnector *conn,
			     struct modeset_dev *dev)
{
	int ret;

	// check if a monitor is connected
	if (conn->connection != DRM_MODE_CONNECTED) {
		printf("ignoring unused connector %u\n",
			conn->connector_id);
		return -ENOENT;
	}

	// check if there is at least one valid mode
	if (conn->count_modes == 0) {
		printf("no valid mode for connector %u\n",
			conn->connector_id);
		return -EFAULT;
	}

	// copy the mode information into our device structure and into both buffers
	memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
	dev->bufs[0].width = conn->modes[0].hdisplay;
	dev->bufs[0].height = conn->modes[0].vdisplay;
	dev->bufs[1].width = conn->modes[0].hdisplay;
	dev->bufs[1].height = conn->modes[0].vdisplay;
	printf("mode for connector %u is %ux%u\n",
		conn->connector_id, dev->bufs[0].width, dev->bufs[0].height);

	// find a crtc for this connector
	ret = modeset_find_crtc(fd, res, conn, dev);
	if (ret) {
		printf("no valid crtc for connector %u\n",
			conn->connector_id);
		return ret;
	}

	// create framebuffer #1 for this CRTC
	ret = modeset_create_fb(fd, &dev->bufs[0]);
	if (ret) {
		printf("cannot create framebuffer for connector %u\n",
			conn->connector_id);
		return ret;
	}

	// create framebuffer #2 for this CRTC
	ret = modeset_create_fb(fd, &dev->bufs[1]);
	if (ret) {
		printf("cannot create framebuffer for connector %u\n",
			conn->connector_id);
		modeset_destroy_fb(fd, &dev->bufs[0]);
		return ret;
	}

	return 0;
}

/*
 * modeset_find_crtc() stays the same.
 */

static int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
				 modeset_dev *dev)
{
	drmModeEncoder *enc;
	unsigned int i, j;
	int32_t crtc;
	struct modeset_dev *iter;

	// first try the currently conected encoder+crtc
	if (conn->encoder_id)
		enc = drmModeGetEncoder(fd, conn->encoder_id);
	else
		enc = NULL;

	if (enc) {
		if (enc->crtc_id) {
			crtc = enc->crtc_id;
			for (iter = dev; iter; iter = iter->next) {
				if (iter->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

	/* If the connector is not currently bound to an encoder or if the
	 * encoder+crtc is already used by another connector (actually unlikely
	 * but lets be safe), iterate all other available encoders to find a
	 * matching CRTC. */
	for (i = 0; i < conn->count_encoders; ++i) {
		enc = drmModeGetEncoder(fd, conn->encoders[i]);
		if (!enc) {
			printf("cannot retrieve encoder %u:%u (%d): %m\n",
				i, conn->encoders[i], errno);
			continue;
		}

		// iterate all global CRTCs
		for (j = 0; j < res->count_crtcs; ++j) {
			// check whether this CRTC works with the encoder
			if (!(enc->possible_crtcs & (1 << j)))
				continue;

			// check that no other device already uses this CRTC
			crtc = res->crtcs[j];
			for (iter = dev; iter; iter = iter->next) {
				if (iter->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			// we have found a CRTC, so save it and return
			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

	printf("cannot find suitable CRTC for connector %u\n",
		conn->connector_id);
	return -ENOENT;
}

/*
 * modeset_create_fb() is mostly the same as before. Buf instead of writing the
 * fields of a modeset_dev, we now require a buffer pointer passed as @buf.
 * Please note that buf->width and buf->height are initialized by
 * modeset_setup_dev() so we can use them here.
 */

static int modeset_create_fb(int fd, struct modeset_buf *buf)
{
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	int ret;

	// create dumb buffer
	memset(&creq, 0, sizeof(creq));
	creq.width = buf->width;
	creq.height = buf->height;
	creq.bpp = 32;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (ret < 0) {
		printf("cannot create dumb buffer (%d): %m\n",
			errno);
		return -errno;
	}
	buf->stride = creq.pitch;
	buf->size = creq.size;
	buf->handle = creq.handle;

	// create framebuffer object for the dumb-buffer
	ret = drmModeAddFB(fd, buf->width, buf->height, 24, 32, buf->stride,
			   buf->handle, &buf->fb);
	if (ret) {
		printf("cannot create framebuffer (%d): %m\n",
			errno);
		ret = -errno;

		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = buf->handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		return ret;
	}

	// prepare buffer for memory mapping
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = buf->handle;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
		printf("cannot map dumb buffer (%d): %m\n",
			errno);
		ret = -errno;

		drmModeRmFB(fd, buf->fb);
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = buf->handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		return ret;
	}

	// perform actual memory mapping
	buf->map = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		        fd, mreq.offset);
	if (buf->map == MAP_FAILED) {
		printf("cannot mmap dumb buffer (%d): %m\n",
			errno);
		ret = -errno;

		drmModeRmFB(fd, buf->fb);
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = buf->handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		return ret;
	}

	// clear the framebuffer to 0
	memset(buf->map, 0, buf->size);

	return 0;
}

/*
 * modeset_destroy_fb() is a new function. It does exactly the reverse of
 * modeset_create_fb() and destroys a single framebuffer. The modeset.c example
 * used to do this directly in modeset_cleanup().
 * We simply unmap the buffer, remove the drm-FB and destroy the memory buffer.
 */

static void modeset_destroy_fb(int fd, struct modeset_buf *buf)
{
	struct drm_mode_destroy_dumb dreq;

	// unmap buffer
	munmap(buf->map, buf->size);

	// delete framebuffer
	drmModeRmFB(fd, buf->fb);

	// delete dumb buffer
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = buf->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

/*
 * modeset_draw() is the place where things change. The render-logic is the same
 * and we still draw a solid-color on the whole screen. However, we now have two
 * buffers and need to flip between them.
 *
 * So before drawing into a framebuffer, we need to find the back-buffer.
 * Remember, dev->font_buf is the index of the front buffer, so
 * dev->front_buf ^ 1 is the index of the back buffer. We simply use
 * dev->bufs[dev->front_buf ^ 1] to get the back-buffer and draw into it.
 *
 * After we finished drawing, we need to flip the buffers. We do this with the
 * same call as we initially set the CRTC: drmModeSetCrtc(). However, we now
 * pass the back-buffer as new framebuffer as we want to flip them.
 * The only thing left to do is to change the dev->front_buf index to point to
 * the new back-buffer (which was previously the front buffer).
 * We then sleep for a short time period and start drawing again.
 *
 * If you run this example, you will notice that there is almost no flickering,
 * anymore. The buffers are now swapped as a whole so each new frame shows
 * always the whole new image. If you look carefully, you will notice that the
 * modeset.c example showed many screen corruptions during redraw-cycles.
 *
 * However, this example is still not perfect. Imagine the display-controller is
 * currently scanning out a new image and we call drmModeSetCrtc()
 * simultaneously. It will then have the same effect as if we used a single
 * buffer and we get some tearing. But, the chance that this happens is a lot
 * less likely as with a single-buffer. This is because there is a long period
 * between each frame called vertical-blank where the display-controller does
 * not perform a scanout. If we swap the buffers in this period, we have the
 * guarantee that there will be no tearing. See the modeset-vsync.c example if
 * you want to know how you can guarantee that the swap takes place at a
 * vertical-sync.
 */

void modeset_swapbuffer(int fd, modeset_dev* dev, unsigned index)
{
	//TODO use index!!

	struct modeset_dev *iter;
	struct modeset_buf *buf;
	int ret;

	for (iter = dev; iter; iter = iter->next) {
		buf = &iter->bufs[iter->front_buf ^ 1];

		ret = drmModeSetCrtc(fd, iter->crtc, buf->fb, 0, 0,
					 &iter->conn, 1, &iter->mode);
		if (ret)
			printf("cannot flip CRTC for connector %u (%d): %m\n",
				iter->conn, errno);
		else
			iter->front_buf ^= 1;
	}
}

/*
 * modeset_cleanup() stays the same as before. But it now calls
 * modeset_destroy_fb() instead of accessing the framebuffers directly.
 */

void modeset_destroy(int fd, modeset_dev* dev)
{
	struct modeset_dev *iter;

	while (dev) {
		// remove from global list
		iter = dev;
		dev = iter->next;

		// restore saved CRTC configuration
		drmModeSetCrtc(fd,
			       iter->saved_crtc->crtc_id,
			       iter->saved_crtc->buffer_id,
			       iter->saved_crtc->x,
			       iter->saved_crtc->y,
			       &iter->conn,
			       1,
			       &iter->saved_crtc->mode);
		drmModeFreeCrtc(iter->saved_crtc);

		// destroy framebuffers
		modeset_destroy_fb(fd, &iter->bufs[1]);
		modeset_destroy_fb(fd, &iter->bufs[0]);

		// free allocated memory
		free(iter);
	}
}

/*
 * This was a very short extension to the basic modesetting example that shows
 * how double-buffering is implemented. Double-buffering is the de-facto
 * standard in any graphics application so any other example will be based on
 * this. It is important to understand the ideas behind it as the code is pretty
 * easy and short compared to modeset.c.
 *
 * Double-buffering doesn't solve all problems. Vsync'ed page-flips solve most
 * of the problems that still occur, but has problems on it's own (see
 * modeset-vsync.c for a discussion).
 *
 * If you want more code, I can recommend reading the source-code of:
 *  - plymouth (which uses dumb-buffers like this example; very easy to understand)
 *  - kmscon (which uses libuterm to do this)
 *  - wayland (very sophisticated DRM renderer; hard to understand fully as it
 *             uses more complicated techniques like DRM planes)
 *  - xserver (very hard to understand as it is split across many files/projects)
 *
 * Any feedback is welcome. Feel free to use this code freely for your own
 * documentation or projects.
 *
 *  - Hosted on http://github.com/dvdhrm/docs
 *  - Written by David Herrmann <dh.herrmann@googlemail.com>
 */
