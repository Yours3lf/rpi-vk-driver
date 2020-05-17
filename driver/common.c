#include "common.h"

#include "kernel/vc4_packet.h"
#include "brcm/cle/v3d_decoder.h"
#include "brcm/clif/clif_dump.h"

uint32_t getFormatBpp(VkFormat f)
{
	switch(f)
	{
	case VK_FORMAT_R64G64B64A64_UINT:
	case VK_FORMAT_R64G64B64A64_SINT:
	case VK_FORMAT_R64G64B64A64_SFLOAT:
		return 256;
	case VK_FORMAT_R64G64B64_UINT:
	case VK_FORMAT_R64G64B64_SINT:
	case VK_FORMAT_R64G64B64_SFLOAT:
		return 192;
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R64G64_UINT:
	case VK_FORMAT_R64G64_SINT:
	case VK_FORMAT_R64G64_SFLOAT:
		return 128;
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
		return 96;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_USCALED:
	case VK_FORMAT_R16G16B16A16_SSCALED:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R64_UINT:
	case VK_FORMAT_R64_SINT:
	case VK_FORMAT_R64_SFLOAT:
		return 64;
	case VK_FORMAT_R16G16B16_UNORM:
	case VK_FORMAT_R16G16B16_SNORM:
	case VK_FORMAT_R16G16B16_USCALED:
	case VK_FORMAT_R16G16B16_SSCALED:
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16B16_SFLOAT:
		return 48;
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R8G8B8A8_UINT:	
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SNORM:
	case VK_FORMAT_B8G8R8A8_USCALED:
	case VK_FORMAT_B8G8R8A8_SSCALED:
	case VK_FORMAT_B8G8R8A8_UINT:
	case VK_FORMAT_B8G8R8A8_SINT:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_USCALED:
	case VK_FORMAT_R16G16_SSCALED:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		return 32;
	case VK_FORMAT_R8G8B8_UNORM:
	case VK_FORMAT_R8G8B8_SNORM:
	case VK_FORMAT_R8G8B8_USCALED:
	case VK_FORMAT_R8G8B8_SSCALED:
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8B8_SINT:
	case VK_FORMAT_R8G8B8_SRGB:
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_B8G8R8_SNORM:
	case VK_FORMAT_B8G8R8_USCALED:
	case VK_FORMAT_B8G8R8_SSCALED:
	case VK_FORMAT_B8G8R8_UINT:
	case VK_FORMAT_B8G8R8_SINT:
	case VK_FORMAT_B8G8R8_SRGB:
		return 24;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8G8_SSCALED:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SRGB:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_USCALED:
	case VK_FORMAT_R16_SSCALED:
	case VK_FORMAT_R16_UINT:
		return 16;
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_S8_UINT:
	case VK_FORMAT_R4G4_UNORM_PACK8:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_R8_SSCALED:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SRGB:
	case VK_FORMAT_UNDEFINED: //TODO
	case VK_FORMAT_G8B8G8R8_422_UNORM: //TODO
		return 8;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return 64; //TODO ???
	case VK_FORMAT_D16_UNORM_S8_UINT:
		return 32; //TODO ???
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		return 32;
	case VK_FORMAT_D16_UNORM:
		return 16;
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: //TODO
		return 4;
	default://
		fprintf(stderr, "format %i\n", f);
		assert(!"Unknown format.");
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

uint32_t isLTformat(uint32_t bpp, uint32_t width, uint32_t height)
{
	uint32_t utileW, utileH;
	getUTileDimensions(bpp, &utileW, &utileH);
	return (width <= 4 * utileW || height <= 4 * utileH);
}

//Textures in T format:
//formed out of 4KB tiles, which have 1KB subtiles (see page 105 in VC4 arch guide)
//1KB subtiles have 512b microtiles.
//Textures in LT format consist of 512b microtiles linearly laid out
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
void getUTileDimensions(uint32_t bpp, uint32_t* tileW, uint32_t* tileH)
{
	assert(tileW);
	assert(tileH)

	switch(bpp)
	{
	case 256:
	{
		*tileW = 1;
		*tileH = 2;
		break;
	}
	case 128:
	{
		*tileW = 2;
		*tileH = 2;
		break;
	}
	case 64:
	{
		*tileW = 2;
		*tileH = 4;
		break;
	}
	case 32:
	case 24: //TODO
	{
		*tileW = 4;
		*tileH = 4;
		break;
	}
	case 16:
	{
		*tileW = 8;
		*tileH = 4;
		break;
	}
	case 8:
	{
		*tileW = 8;
		*tileH = 8;
		break;
	}
	case 4:
	{
		*tileW = 16;
		*tileH = 8;
		break;
	}
	case 1:
	{
		*tileW = 32;
		*tileH = 16;
		break;
	}
	default:
	{
		fprintf(stderr, "bpp: %i\n", bpp);
		assert(!"Unsupported texture bpp.");
	}
	}
}

uint32_t roundUp(uint32_t numToRound, uint32_t multiple)
{
	if(!multiple)
	{
		return numToRound;
	}

	uint32_t remainder = numToRound % multiple;

	if(!remainder)
	{
		return numToRound;
	}

	return numToRound + multiple - remainder;
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

int isDepthStencilFormat(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_S8_UINT:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return 1;
	default:
		return 0;
	}
}

uint32_t getCompareOp(VkCompareOp op)
{
	switch(op)
	{
	case VK_COMPARE_OP_NEVER:
		return V3D_COMPARE_FUNC_NEVER;
	case VK_COMPARE_OP_LESS:
		return V3D_COMPARE_FUNC_LESS;
	case VK_COMPARE_OP_EQUAL:
		return V3D_COMPARE_FUNC_EQUAL;
	case VK_COMPARE_OP_LESS_OR_EQUAL:
		return V3D_COMPARE_FUNC_LEQUAL;
	case VK_COMPARE_OP_GREATER:
		return V3D_COMPARE_FUNC_GREATER;
	case VK_COMPARE_OP_NOT_EQUAL:
		return V3D_COMPARE_FUNC_NOTEQUAL;
	case VK_COMPARE_OP_GREATER_OR_EQUAL:
		return V3D_COMPARE_FUNC_GEQUAL;
	case VK_COMPARE_OP_ALWAYS:
		return V3D_COMPARE_FUNC_ALWAYS;
	default:
		return -1;
	}
}

uint32_t getStencilOp(VkStencilOp op)
{
	switch(op)
	{
	case VK_STENCIL_OP_ZERO:
		return 0;
	case VK_STENCIL_OP_KEEP:
		return 1;
	case VK_STENCIL_OP_REPLACE:
		return 2;
	case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
		return 3;
	case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
		return 4;
	case VK_STENCIL_OP_INVERT:
		return 5;
	case VK_STENCIL_OP_INCREMENT_AND_WRAP:
		return 6;
	case VK_STENCIL_OP_DECREMENT_AND_WRAP:
		return 7;
	default:
		return -1;
	};
}

uint32_t getTopology(VkPrimitiveTopology topology)
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return 0;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return 1;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return 2;
	default:
		return -1;
	}
}

uint32_t getPrimitiveMode(VkPrimitiveTopology topology)
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return V3D_PRIM_POINTS;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return V3D_PRIM_LINES;
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return V3D_PRIM_LINE_STRIP;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return V3D_PRIM_TRIANGLES;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return V3D_PRIM_TRIANGLE_STRIP;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return V3D_PRIM_TRIANGLE_FAN;
	default:
		return -1;
	}
}

uint32_t ulog2(uint32_t v)
{
	uint32_t ret = 0;
	while(v >>= 1) ret++;
	return ret;
}

void clFit(VkCommandBuffer cb, ControlList* cl, uint32_t commandSize)
{
	if(!clHasEnoughSpace(cl, commandSize))
	{
		uint32_t currSize = cl->nextFreeByteOffset - cl->offset;
		uint32_t currMarkerOffset = cl->currMarkerOffset - cl->offset;
		cl->offset = consecutivePoolReAllocate(cl->CPA, getCPAptrFromOffset(cl->CPA, cl->offset), cl->numBlocks); assert(cl->offset != -1);
		cl->nextFreeByteOffset = cl->offset + currSize;
		cl->numBlocks++;
		cl->currMarkerOffset = cl->currMarkerOffset == -1 ? -1 : cl->offset + currMarkerOffset;
		if(cl->currMarkerOffset != -1)
		{
			assert(((CLMarker*)getCPAptrFromOffset(cl->CPA, cl->currMarkerOffset))->memGuard == 0xDDDDDDDD);
		}
	}
}

void clDump(void* cl, uint32_t size)
{
		struct v3d_device_info devinfo = {
				/* While the driver supports V3D 2.1 and 2.6, we haven't split
				 * off a 2.6 XML yet (there are a couple of fields different
				 * in render target formatting)
				 */
				.ver = 21,
		};
		struct v3d_spec* spec = v3d_spec_load(&devinfo);

		struct clif_dump *clif = clif_dump_init(&devinfo, stderr, true);

		uint32_t offset = 0, hw_offset = 0;
		uint8_t *p = cl;

		while (offset < size) {
				struct v3d_group *inst = v3d_spec_find_instruction(spec, p);
				uint8_t header = *p;
				uint32_t length;

				if (inst == NULL) {
						fprintf(stderr, "0x%08x 0x%08x: Unknown packet 0x%02x (%d)!\n",
								offset, hw_offset, header, header);
						return;
				}

				length = v3d_group_get_length(inst);

				printf("0x%08x 0x%08x: 0x%02x %s\n",
						offset, hw_offset, header, v3d_group_get_name(inst));

				v3d_print_group(clif, inst, offset, p);

				switch (header) {
				case VC4_PACKET_HALT:
				case VC4_PACKET_STORE_MS_TILE_BUFFER_AND_EOF:
						return;
				default:
						break;
				}

				offset += length;
				if (header != VC4_PACKET_GEM_HANDLES)
						hw_offset += length;
				p += length;
		}

		clif_dump_destroy(clif);
}

void encodeTextureUniform(uint32_t* params, //array of 4 uint32_t
						  //num mip levels - 1
						  uint8_t numMipLevels,
						  ///0:  rgba8
						  ///1:  rgbx8 (a=1)
						  ///2:  rgba4
						  ///3:  rgb5a1
						  ///4:  r5g6b5 (a=1)
						  ///5:  luminance (8 bit, a=1)
						  //6:  alpha (8 bit, rga=0)
						  ///7:  lumalpha
						  ///8:  etc1
						  ///9:  s16f (blending supported)
						  ///10: s8 (blending supported)
						  ///11: s16 (point sampling only)
						  //12: bw1 (1 bit black and white)
						  //13: a4
						  //14: a1
						  ///15: rgba16f
						  //16: rgba8r (raster format = not in T format)
						  ///17: yuyv422r (raster format = not in T format, yuyv)
						  uint8_t textureDataType,
						  uint8_t isCubeMap,
						  uint32_t cubemapStride, //in multiples of 4k bytes
						  uint32_t textureBasePtr, //in multiples of 4k bytes
						  //0 = 2048
						  uint16_t height,
						  uint16_t width,
						  //0: linear
						  //1: nearest
						  //2: near_mip_near
						  //3: near_mip_lin
						  //4: lin_mip_near
						  //5: lin_mip_lin
						  uint8_t minFilter,
						  //0: linear
						  //1: nearest
						  uint8_t magFilter,
						  //0: repeat
						  //1: clamp
						  //2: mirror
						  //3: border
						  uint8_t wrapT,
						  uint8_t wrapS,
						  uint8_t noAutoLod //disable automatic LOD, use bias only
						  )
{
	assert(params);

	params[0] = 0
			| (numMipLevels & 0xf)
			| (uint32_t)(textureDataType & 0xf) << 4
			| (uint32_t)(isCubeMap ? 1 : 0) << 9
			| (uint32_t)(textureBasePtr & 0xfffff) << 12;

	params[1] = 0
			| (wrapS & 0x3)
			| (uint32_t)(wrapT & 0x3) << 2
			| (uint32_t)(minFilter & 0x7) << 4
			| (uint32_t)(magFilter & 0x1) << 7
			| (uint32_t)(width & 0x7ff) << 8
			| (uint32_t)(height & 0x7ff) << 20
			| (uint32_t)((textureDataType & 0x10) >> 4) << 31;

	params[2] = 0
			| (noAutoLod & 0x1)
			| (uint32_t)(cubemapStride & 0x3ffff) << 12
			| (uint32_t)(isCubeMap || noAutoLod ? 1 : 0) << 30;

	//TODO
	//child images
	params[3] = 0;
}

void encodeStencilValue(uint32_t *values, uint32_t* numValues, VkStencilOpState front, VkStencilOpState back, uint8_t stencilTestEnable)
{
	assert(values);
	assert(numValues);

	if(!stencilTestEnable)
	{
		front.compareOp = back.compareOp = VK_COMPARE_OP_ALWAYS;
	}

	if(front.compareMask == back.compareMask &&
	   front.compareOp == back.compareOp &&
	   front.depthFailOp == back.depthFailOp &&
	   front.failOp == back.failOp &&
	   front.passOp == back.passOp &&
	   front.reference == back.reference &&
	   front.writeMask == back.writeMask
	   )
	{
		*numValues = 1;

		values[0] = 0
				| (front.compareMask & 0xff)
				| (front.reference & 0xff) << 0x8
				| (getCompareOp(front.compareOp) & 0x7) << 16
				| (getStencilOp(front.failOp) & 0x7) << 19
				| (getStencilOp(front.passOp) & 0x7) << 22
				| (getStencilOp(front.depthFailOp) & 0x7) << 25
				| 3 << 30; //front and back

		switch(front.writeMask)
		{
		case 0x1:
			values[0] |= 0 << 28;
			break;
		case 0x3:
			values[0] |= 1 << 28;
			break;
		case 0xf:
			values[0] |= 2 << 28;
			break;
		case 0xff:
			values[0] |= 3 << 28;
			break;
		default:
			values[1] = 0
					| (front.writeMask & 0xff)
					| (front.writeMask & 0xff) << 8;
			*numValues = 2;
			break;
		};
	}
	else
	{
		*numValues = 2;

		values[0] = 0
				| (front.compareMask & 0xff)
				| (front.reference & 0xff) << 0x8
				| (getCompareOp(front.compareOp) & 0x7) << 16
				| (getStencilOp(front.failOp) & 0x7) << 19
				| (getStencilOp(front.passOp) & 0x7) << 22
				| (getStencilOp(front.depthFailOp) & 0x7) << 25
				| 1 << 30; //front

		values[1] = 0
				| (back.compareMask & 0xff)
				| (back.reference & 0xff) << 0x8
				| (getCompareOp(back.compareOp) & 0x7) << 16
				| (getStencilOp(back.failOp) & 0x7) << 19
				| (getStencilOp(back.passOp) & 0x7) << 22
				| (getStencilOp(back.depthFailOp) & 0x7) << 25
				| 2 << 30; //front

		if((front.writeMask == 0x1 ||
		   front.writeMask == 0x3 ||
		   front.writeMask == 0xf ||
		   front.writeMask == 0xff) &&
		   (back.writeMask == 0x1 ||
		   back.writeMask == 0x3 ||
		   back.writeMask == 0xf ||
		   back.writeMask == 0xff))
		{
			switch(front.writeMask)
			{
			case 0x1:
				values[0] |= 0 << 28;
				break;
			case 0x3:
				values[0] |= 1 << 28;
				break;
			case 0xf:
				values[0] |= 2 << 28;
				break;
			case 0xff:
				values[0] |= 3 << 28;
				break;
			};

			switch(back.writeMask)
			{
			case 0x1:
				values[1] |= 0 << 28;
				break;
			case 0x3:
				values[1] |= 1 << 28;
				break;
			case 0xf:
				values[1] |= 2 << 28;
				break;
			case 0xff:
				values[1] |= 3 << 28;
				break;
			};
		}
		else
		{
			values[2] = 0
					| (front.writeMask & 0xff)
					| (back.writeMask & 0xff) << 8;
			*numValues = 3;
		}
	}
}

uint32_t encodeVPMSetup(uint8_t stride,
						uint8_t direction, //0 vertical, 1 horizontal
						uint8_t isLaned, //0 packed, 1 laned
						uint8_t size, //0 8bit, 1 16bit, 2 32bit
						uint8_t address, //see doc
						uint8_t vectorComponentsToRead //only used for VPM read setup
							 )
{
	uint32_t res = 0;
	res |= ((uint32_t)(vectorComponentsToRead) & 0xf) << 20;
	res |= ((uint32_t)(stride) & 0x3f) << 12;
	res |= ((uint32_t)(direction) & 0x1) << 11;
	res |= ((uint32_t)(isLaned) & 0x1) << 10;
	res |= ((uint32_t)(size) & 0x3) << 8;
	res |= (uint32_t)(address) & 0xff;

	return res;
}

uint8_t getTextureDataType(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 15; //rgba16f
	case VK_FORMAT_R8G8B8_UNORM:
		return 1; //rgbx8 (a=1)
	case VK_FORMAT_R8G8B8A8_UNORM:
		return 0; //rgba8
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		return 3; //rgb5a1
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		return 2; //rgba4
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		return 4; //b5g6r5 (a=1)
	case VK_FORMAT_R8G8_UNORM:
		return 7; //lumalpha
	case VK_FORMAT_R16_SFLOAT:
		return 9; //s16f (blending supported)
	case VK_FORMAT_R16_SINT:
		return 11; //s16 (point sampling only)
	case VK_FORMAT_R8_UNORM:
		return 5; //luminance (8 bit, a=1)
	case VK_FORMAT_R8_SINT:
		return 10; //s8 (blending supported)
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		return 8; //etc1
	case VK_FORMAT_G8B8G8R8_422_UNORM:
		return 17; //yuyv422r (raster format = not in T format, yuyv)
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return 0; //rgba8
	case VK_FORMAT_UNDEFINED: //TODO
		return -1;
	default://
		fprintf(stderr, "format %i\n", format);
		assert(!"Unsupported format.");
		return -1;
	}
}

uint8_t getMinFilterType(VkFilter minFilter, VkSamplerMipmapMode mipFilter)//, float maxLod)
{
	if(minFilter == VK_FILTER_NEAREST)
	{
//		if(maxLod < 0.0001f)
//		{
//			return 1; //no mip filtering
//		}

		if(mipFilter == VK_SAMPLER_MIPMAP_MODE_NEAREST)
		{
			return 2;
		}
		else if(mipFilter == VK_SAMPLER_MIPMAP_MODE_LINEAR)
		{
			return 3;
		}
	}
	else if(minFilter == VK_FILTER_LINEAR)
	{
//		if(maxLod < 0.0001f)
//		{
//			return 0; //no mip filtering
//		}

		if(mipFilter == VK_SAMPLER_MIPMAP_MODE_NEAREST)
		{
			return 4;
		}
		else if(mipFilter == VK_SAMPLER_MIPMAP_MODE_LINEAR)
		{
			return 5;
		}
	}

	return -1;
}

uint8_t getWrapMode(VkSamplerAddressMode mode)
{
	if(mode == VK_SAMPLER_ADDRESS_MODE_REPEAT)
	{
		return 0;
	}
	else if(mode == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	{
		return 1;
	}
	else if(mode == VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT)
	{
		return 2;
	}
	else if(mode == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
	{
		return 3;
	}
	else
	{
		fprintf(stderr, "wrap mode: %i\n", mode);
		assert(!"Unsupported wrap mode.");
		return -1;
	}
}

uint32_t getRenderTargetFormatVC4(VkFormat format)
{
	//TODO dithered BGR565
	switch(format)
	{
		case VK_FORMAT_R16G16B16A16_SFLOAT: //HDR mode set in tile binning config mode, so just return a valid format
		case VK_FORMAT_R8G8B8A8_UNORM:
		//only here so we can do emulated buffer copies to depth textures
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		//only here so we can copy ETC1 textures to optimal format
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return VC4_RENDER_CONFIG_FORMAT_RGBA8888;
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
		//TODO
		//case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		//case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		//case VK_FORMAT_R8G8_UNORM:
		//case VK_FORMAT_R16_SFLOAT:
		//case VK_FORMAT_R16_SINT:
			return VC4_RENDER_CONFIG_FORMAT_BGR565;
		default:
			fprintf(stderr, "rendertarget format: %i\n", format);
			assert(!"Unsupported render target format");
			return -1;
	}
}

//return closest power of 2 number greater or equal to n
uint32_t getPow2Pad(uint32_t n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return ++n;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
/// just so we can return a function pointer
////////////////////////////////////////////////////
////////////////////////////////////////////////////

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalBufferProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
	VkExternalBufferProperties*                 pExternalBufferProperties)
{
	UNSUPPORTED(vkGetPhysicalDeviceExternalBufferProperties);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalFenceProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
	VkExternalFenceProperties*                  pExternalFenceProperties)
{
	UNSUPPORTED(vkGetPhysicalDeviceExternalFenceProperties);
}


VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalSemaphoreProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
	VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
	UNSUPPORTED(vkGetPhysicalDeviceExternalSemaphoreProperties);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceGroupPeerMemoryFeatures(
	VkDevice                                    device,
	uint32_t                                    heapIndex,
	uint32_t                                    localDeviceIndex,
	uint32_t                                    remoteDeviceIndex,
	VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures)
{
	UNSUPPORTED(vkGetDeviceGroupPeerMemoryFeatures);
}
