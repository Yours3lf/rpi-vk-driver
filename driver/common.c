#include "common.h"

#include "kernel/vc4_packet.h"

void createImageBO(_image* i)
{
	assert(i);
	assert(i->format);
	assert(i->width);
	assert(i->height);

	uint32_t bpp = getFormatBpp(i->format);
	uint32_t pixelSizeBytes = bpp / 8;
	uint32_t nonPaddedSize = i->width * i->height * pixelSizeBytes;
	i->paddedWidth = i->width;
	i->paddedHeight = i->height;

	//need to pad to T format, as HW automatically chooses that
	if(nonPaddedSize > 4096)
	{
		getPaddedTextureDimensionsT(i->width, i->height, bpp, &i->paddedWidth, &i->paddedHeight);
	}

	i->size = i->paddedWidth * i->paddedHeight * pixelSizeBytes;
	i->stride = i->paddedWidth * pixelSizeBytes;
	i->handle = vc4_bo_alloc(controlFd, i->size, "swapchain image"); assert(i->handle);

	//set tiling to T if size > 4KB
	if(nonPaddedSize > 4096)
	{
		int ret = vc4_bo_set_tiling(controlFd, i->handle, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED); assert(ret);
		i->tiling = VC4_TILING_FORMAT_T;
	}
	else
	{
		int ret = vc4_bo_set_tiling(controlFd, i->handle, DRM_FORMAT_MOD_LINEAR); assert(ret);
		i->tiling = VC4_TILING_FORMAT_LT;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearColorImage
 * Color and depth/stencil images can be cleared outside a render pass instance using vkCmdClearColorImage or vkCmdClearDepthStencilImage, respectively.
 * These commands are only allowed outside of a render pass instance.
 */
VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(
		VkCommandBuffer                             commandBuffer,
		VkImage                                     image,
		VkImageLayout                               imageLayout,
		const VkClearColorValue*                    pColor,
		uint32_t                                    rangeCount,
		const VkImageSubresourceRange*              pRanges)
{
	assert(commandBuffer);
	assert(image);
	assert(pColor);

	//TODO this should only flag an image for clearing. This can only be called outside a renderpass
	//actual clearing would only happen:
	// -if image is rendered to (insert clear before first draw call)
	// -if the image is bound for sampling (submit a CL with a clear)
	// -if a command buffer is submitted without any rendering (insert clear)
	// -etc.
	//we shouldn't clear an image if noone uses it

	//TODO ranges support

	assert(imageLayout == VK_IMAGE_LAYOUT_GENERAL ||
		   imageLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR ||
		   imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	assert(commandBuffer->state	 == CMDBUF_STATE_RECORDING);
	assert(_queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT || _queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT);

	_image* i = image;

	assert(i->usageBits & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	//TODO externally sync cmdbuf, cmdpool

	i->needToClear = 1;
	i->clearColor[0] = i->clearColor[1] = packVec4IntoABGR8(pColor->float32);
}

int findInstanceExtension(char* name)
{
	for(int c = 0; c < numInstanceExtensions; ++c)
	{
		if(strcmp(instanceExtensions[c].extensionName, name) == 0)
		{
			return c;
		}
	}

	return -1;
}

int findDeviceExtension(char* name)
{
	for(int c = 0; c < numDeviceExtensions; ++c)
	{
		if(strcmp(deviceExtensions[c].extensionName, name) == 0)
		{
			return c;
		}
	}

	return -1;
}

//Textures in T format:
//formed out of 4KB tiles, which have 1KB subtiles (see page 105 in VC4 arch guide)
//1KB subtiles have 512b microtiles.
//Width/height of the 512b microtiles is the following:
// 64bpp: 2x4
// 32bpp: 4x4
// 16bpp: 8x4
// 8bpp:  8x8
// 4bpp:  16x8
// 1bpp:  32x16
//Therefore width/height of 1KB subtiles is the following:
// 64bpp: 8x16
// 32bpp: 16x16
// 16bpp: 32x16
// 8bpp:  32x32
// 4bpp:  64x32
// 1bpp:  128x64
//Finally width/height of the 4KB tiles:
// 64bpp: 16x32
// 32bpp: 32x32
// 16bpp: 64x32
// 8bpp:  64x64
// 4bpp:  128x64
// 1bpp:  256x128
void getPaddedTextureDimensionsT(uint32_t width, uint32_t height, uint32_t bpp, uint32_t* paddedWidth, uint32_t* paddedHeight)
{
	assert(paddedWidth);
	assert(paddedHeight);
	uint32_t tileW = 0;
	uint32_t tileH = 0;

	switch(bpp)
	{
	case 64:
	{
		tileW = 16;
		tileH = 32;
		break;
	}
	case 32:
	{
		tileW = 32;
		tileH = 32;
		break;
	}
	case 16:
	{
		tileW = 64;
		tileH = 32;
		break;
	}
	case 8:
	{
		tileW = 64;
		tileH = 64;
		break;
	}
	case 4:
	{
		tileW = 128;
		tileH = 64;
		break;
	}
	case 1:
	{
		tileW = 256;
		tileH = 128;
		break;
	}
	default:
	{
		assert(0); //unsupported
	}
	}

	*paddedWidth = ((tileW - (width % tileW)) % tileW) + width;
	*paddedHeight = ((tileH - (height % tileH)) % tileH) + height;
}

uint32_t getFormatBpp(VkFormat f)
{
	switch(f)
	{
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 64;
	case VK_FORMAT_R8G8B8_UNORM: //padded to 32
	case VK_FORMAT_R8G8B8A8_UNORM:
		return 32;
		return 32;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16_SINT:
		return 16;
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SINT:
		return 8;
	default:
		assert(0);
		return 0;
	}
}

uint32_t packVec4IntoABGR8(const float rgba[4])
{
	uint8_t r, g, b, a;
	r = rgba[0] * 255.0;
	g = rgba[1] * 255.0;
	b = rgba[2] * 255.0;
	a = rgba[3] * 255.0;

	uint32_t res = 0 |
			(a << 24) |
			(b << 16) |
			(g << 8) |
			(r << 0);

	return res;
}

/*static inline void util_pack_color(const float rgba[4], enum pipe_format format, union util_color *uc)
{
   ubyte r = 0;
   ubyte g = 0;
   ubyte b = 0;
   ubyte a = 0;

   if (util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0) <= 8) {
	  r = float_to_ubyte(rgba[0]);
	  g = float_to_ubyte(rgba[1]);
	  b = float_to_ubyte(rgba[2]);
	  a = float_to_ubyte(rgba[3]);
   }

   switch (format) {
   case PIPE_FORMAT_ABGR8888_UNORM:
	  {
		 uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | a;
	  }
	  return;
   case PIPE_FORMAT_XBGR8888_UNORM:
	  {
		 uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | 0xff;
	  }
	  return;
   case PIPE_FORMAT_BGRA8888_UNORM:
	  {
		 uc->ui[0] = (a << 24) | (r << 16) | (g << 8) | b;
	  }
	  return;
   case PIPE_FORMAT_BGRX8888_UNORM:
	  {
		 uc->ui[0] = (0xffu << 24) | (r << 16) | (g << 8) | b;
	  }
	  return;
   case PIPE_FORMAT_ARGB8888_UNORM:
	  {
		 uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | a;
	  }
	  return;
   case PIPE_FORMAT_XRGB8888_UNORM:
	  {
		 uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | 0xff;
	  }
	  return;
   case PIPE_FORMAT_B5G6R5_UNORM:
	  {
		 uc->us = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
	  {
		 uc->us = ((0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
	  {
		 uc->us = ((a & 0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
	  }
	  return;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
	  {
		 uc->us = ((a & 0xf0) << 8) | ((r & 0xf0) << 4) | ((g & 0xf0) << 0) | (b >> 4);
	  }
	  return;
   case PIPE_FORMAT_A8_UNORM:
	  {
		 uc->ub = a;
	  }
	  return;
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
	  {
		 uc->ub = r;
	  }
	  return;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
	  {
		 uc->f[0] = rgba[0];
		 uc->f[1] = rgba[1];
		 uc->f[2] = rgba[2];
		 uc->f[3] = rgba[3];
	  }
	  return;
   case PIPE_FORMAT_R32G32B32_FLOAT:
	  {
		 uc->f[0] = rgba[0];
		 uc->f[1] = rgba[1];
		 uc->f[2] = rgba[2];
	  }
	  return;

   default:
	  util_format_write_4f(format, rgba, 0, uc, 0, 0, 0, 1, 1);
   }
}*/
