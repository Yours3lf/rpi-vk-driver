#include "modeset.h"

static int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
							 struct modeset_dev *dev);
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

	// retrieve resources
	res = drmModeGetResources(fd);
	if (!res) {
		fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n", errno);
		return 0;
	}

	// iterate all connectors
	for (unsigned i = 0; i < res->count_connectors; ++i) {
		// get information for each connector
		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn) {
			fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n", i, res->connectors[i], errno);
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
				fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n", i, res->connectors[i], errno);
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

	return ret_dev;
}

int modeset_fb_for_dev(int fd, modeset_dev* dev, _image* buffer)
{
	int ret;

	struct modeset_dev *iter;
	//struct modeset_buf *buf;
	for (iter = dev; iter; iter = iter->next) {
		iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
		ret = drmModeSetCrtc(fd, iter->crtc, buffer->fb, 0, 0,
							 &iter->conn, 1, &iter->mode);
		if (ret)
			fprintf(stderr, "cannot set CRTC for connector %u (%d): %m\n",
				   iter->conn, errno);
	}

	return 0;
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
		fprintf(stderr, "ignoring unused connector %u\n",
			   conn->connector_id);
		return -ENOENT;
	}

	// check if there is at least one valid mode
	if (conn->count_modes == 0) {
		fprintf(stderr, "no valid mode for connector %u\n",
			   conn->connector_id);
		return -EFAULT;
	}

	// copy the mode information into our device structure and into both buffers
	memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
	dev->width = conn->modes[0].hdisplay;
	dev->height = conn->modes[0].vdisplay;
	printf("mode for connector %u is %ux%u\n",
		   conn->connector_id, dev->width, dev->height);

	// find a crtc for this connector
	ret = modeset_find_crtc(fd, res, conn, dev);
	if (ret) {
		fprintf(stderr, "no valid crtc for connector %u\n",
			   conn->connector_id);
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
			fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n",
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

	fprintf(stderr, "cannot find suitable CRTC for connector %u\n",
		   conn->connector_id);
	return -ENOENT;
}

/*
 * modeset_create_fb() is mostly the same as before. Buf instead of writing the
 * fields of a modeset_dev, we now require a buffer pointer passed as @buf.
 * Please note that buf->width and buf->height are initialized by
 * modeset_setup_dev() so we can use them here.
 */

int modeset_create_fb(int fd, _image *buf)
{
	int ret;

	// create framebuffer object for the dumb-buffer
	ret = drmModeAddFB(fd, buf->width, buf->height, 24, 32, buf->stride,
					   buf->boundMem->bo, &buf->fb);
	if (ret) {
		fprintf(stderr, "cannot create framebuffer (%d): %m\n",
			   errno);
		ret = -errno;

		return ret;
	}

	return 0;
}

/*
 * modeset_destroy_fb() is a new function. It does exactly the reverse of
 * modeset_create_fb() and destroys a single framebuffer. The modeset.c example
 * used to do this directly in modeset_cleanup().
 * We simply unmap the buffer, remove the drm-FB and destroy the memory buffer.
 */

void modeset_destroy_fb(int fd, _image* buf)
{
	// delete framebuffer
	drmModeRmFB(fd, buf->fb);
}

void modeset_present_buffer(int fd, modeset_dev* dev, _image* buffer)
{
	//TODO use index!!

	if(!dev->saved_crtc)
	{
		int res = modeset_fb_for_dev(fd, dev, buffer); assert(res == 0);
	}

	struct modeset_dev *iter;
	int ret;

	for (iter = dev; iter; iter = iter->next)
	{
		ret = drmModeSetCrtc(fd, iter->crtc, buffer->fb, 0, 0,
							 &iter->conn, 1, &iter->mode);
		if (ret)
			fprintf(stderr, "cannot flip CRTC for connector %u (%d): %m\n",
				   iter->conn, errno);
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

		// free allocated memory
		free(iter);
	}
}





void modeset_enum_displays(int fd, uint32_t* numDisplays, modeset_display* displays)
{
	 drmModeResPtr resPtr = drmModeGetResources(fd);

	 uint32_t tmpNumDisplays = 0;
	 modeset_display tmpDisplays[16];

	 for(uint32_t c = 0; c < resPtr->count_connectors; ++c)
	 {
		 drmModeConnectorPtr connPtr = drmModeGetConnector(fd, resPtr->connectors[c]);

		 if(connPtr->connection != DRM_MODE_CONNECTED)
		 {
			 continue; //skip unused connector
		 }

		 if(!connPtr->count_modes)
		 {
			 continue; //skip connectors with no valid modes
		 }

		 memcpy(tmpDisplays[tmpNumDisplays].name, connPtr->modes[0].name, 32);

		 tmpDisplays[tmpNumDisplays].mmWidth = connPtr->mmWidth;
		 tmpDisplays[tmpNumDisplays].mmHeight = connPtr->mmHeight;
		 tmpDisplays[tmpNumDisplays].resWidth = connPtr->modes[0].hdisplay;
		 tmpDisplays[tmpNumDisplays].resHeight = connPtr->modes[0].vdisplay;
		 tmpDisplays[tmpNumDisplays].connectorID = connPtr->connector_id;

		 tmpNumDisplays++;

		 assert(tmpNumDisplays < 16);

		 drmModeFreeConnector(connPtr);
	 }

	 drmModeFreeResources(resPtr);

	 *numDisplays = tmpNumDisplays;

	 memcpy(displays, tmpDisplays, tmpNumDisplays * sizeof(modeset_display));
}

void modeset_enum_modes_for_display(int fd, uint32_t display, uint32_t* numModes, modeset_display_mode* modes)
{
	drmModeResPtr resPtr = drmModeGetResources(fd);

	drmModeConnectorPtr connPtr = drmModeGetConnector(fd, display);

	uint32_t tmpNumModes = 0;
	modeset_display_mode tmpModes[1024];

	for(uint32_t c = 0; c < connPtr->count_modes; ++c)
	{
		tmpModes[tmpNumModes].connectorID = display;
		tmpModes[tmpNumModes].modeID = c;
		tmpModes[tmpNumModes].refreshRate = connPtr->modes[c].vrefresh;
		tmpModes[tmpNumModes].resWidth = connPtr->modes[c].hdisplay;
		tmpModes[tmpNumModes].resHeight = connPtr->modes[c].vdisplay;

		tmpNumModes++;

		assert(tmpNumModes < 1024);
	}

	drmModeFreeConnector(connPtr);
	drmModeFreeResources(resPtr);

	*numModes = tmpNumModes;
	memcpy(modes, tmpModes, tmpNumModes * sizeof(modeset_display_mode));
}

void modeset_create_surface_for_mode(int fd, uint32_t display, uint32_t mode, modeset_display_surface* surface)
{
	drmModeResPtr resPtr = drmModeGetResources(fd);

	drmModeConnectorPtr connPtr = drmModeGetConnector(fd, display);

	drmModeEncoderPtr encPtr = 0;

	//if current encoder is valid, try to use that
	if(connPtr->encoder_id)
	{
		encPtr = drmModeGetEncoder(fd, connPtr->encoder_id);
	}

	if(encPtr)
	{
		if(encPtr->crtc_id)
		{
			surface->connectorID = display;
			surface->modeID = mode;
			surface->encoderID = connPtr->encoder_id;
			surface->crtcID = encPtr->crtc_id;
		}
	}
}
