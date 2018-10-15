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

	i->size = getBOAlignedSize(i->paddedWidth * i->paddedHeight * pixelSizeBytes);
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

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBeginRenderPass
 */
void vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
	assert(commandBuffer);
	assert(pRenderPassBegin);


	//TODO subpass contents ignored

	//TODO add state tracking to command buffer
	//only bake into control list when a draw call is issued or similar
	_commandBuffer* cb = commandBuffer;
	cb->fbo = pRenderPassBegin->framebuffer;
	cb->renderpass = pRenderPassBegin->renderPass;
	cb->renderArea = pRenderPassBegin->renderArea;

	for(int c = 0; c < pRenderPassBegin->clearValueCount; ++c)
	{
		if(cb->renderpass->attachments[c].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			cb->fbo->attachmentViews[c].image->needToClear = 1;
			cb->fbo->attachmentViews[c].image->clearColor[0] = cb->fbo->attachmentViews[c].image->clearColor[1] = packVec4IntoABGR8(pRenderPassBegin->pClearValues->color.float32);
		}
		else if(isDepthStencilFormat(cb->renderpass->attachments[c].format) && cb->renderpass->attachments[c].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			//TODO how to pack depth/stencil clear values???
			//cb->fbo->attachmentViews[c].image->needToClear = 1;
			//cb->fbo->attachmentViews[c].image->clearColor = packVec4IntoABGR8(pRenderPassBegin->pClearValues->depthStencil.depth);
		}
	}

	cb->currentSubpass = 0;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindPipeline
 */
void vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	if(pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		cb->graphicsPipeline = pipeline;
	}
	else if(pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE)
	{
		cb->computePipeline = pipeline;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetViewport
 */
void vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetScissor
 */
void vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	//TODO
}

uint32_t getDepthCompareOp(VkCompareOp op)
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

uint32_t getFormatByteSize(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_R16_SFLOAT:
		return 2;
	case VK_FORMAT_R16G16_SFLOAT:
		return 4;
	case VK_FORMAT_R16G16B16_SFLOAT:
		return 6;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 8;
	case VK_FORMAT_R32_SFLOAT:
		return 4;
	case VK_FORMAT_R32G32_SFLOAT:
		return 8;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return 8;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return 8;
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

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdDraw
 */
void vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	_renderpass* rp = cb->renderpass;
	_framebuffer* fb = cb->fbo;

	//TODO handle multiple attachments etc.
	_image* i = fb->attachmentViews[rp->subpasses[cb->currentSubpass].pColorAttachments[0].attachment].image;


	//stuff needed to submit a draw call:
	//Tile Binning Mode Configuration
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
	clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
										 0, 0, 0, 0,
										 getFormatBpp(i->format) == 64, //64 bit color mode
										 i->samples > 1, //msaa
										 i->width, i->height, 0, 0, 0);

	//Start Tile Binning
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
	clInsertStartTileBinning(&commandBuffer->binCl);

	//Primitive List Format
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_PRIMITIVE_LIST_FORMAT_length);
	clInsertPrimitiveListFormat(&commandBuffer->binCl,
								1, //16 bit
								getTopology(cb->graphicsPipeline->topology)); //tris

	//Clip Window
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIP_WINDOW_length);
	clInsertClipWindow(&commandBuffer->binCl, i->width, i->height, 0, 0);

	//Configuration Bits
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CONFIGURATION_BITS_length);
	clInsertConfigurationBits(&commandBuffer->binCl,
							  1, //TODO earlyz updates
							  0, //TODO earlyz enable
							  0, //TODO z updates
							  cb->graphicsPipeline->depthTestEnable ? getDepthCompareOp(cb->graphicsPipeline->depthCompareOp) : V3D_COMPARE_FUNC_ALWAYS, //depth compare func
							  0,
							  0,
							  0,
							  0,
							  0,
							  cb->graphicsPipeline->depthBiasEnable, //depth offset enable
							  cb->graphicsPipeline->frontFace == VK_FRONT_FACE_CLOCKWISE, //clockwise
							  !(cb->graphicsPipeline->cullMode & VK_CULL_MODE_BACK_BIT), //enable back facing primitives
							  !(cb->graphicsPipeline->cullMode & VK_CULL_MODE_FRONT_BIT)); //enable front facing primitives

	//TODO Depth Offset
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_DEPTH_OFFSET_length);
	clInsertDepthOffset(&commandBuffer->binCl, cb->graphicsPipeline->depthBiasConstantFactor, cb->graphicsPipeline->depthBiasSlopeFactor);

	//Point size
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_POINT_SIZE_length);
	clInsertPointSize(&commandBuffer->binCl, 1.0f);

	//Line width
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_LINE_WIDTH_length);
	clInsertLineWidth(&commandBuffer->binCl, cb->graphicsPipeline->lineWidth);

	//TODO why flipped???
	//Clipper XY Scaling
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_XY_SCALING_length);
	clInsertClipperXYScaling(&commandBuffer->binCl, (float)(i->width) * 0.5f * 16.0f, -1.0f * (float)(i->height) * 0.5f * 16.0f);

	//TODO how is this calculated?
	//seems to go from -1.0 .. 1.0 to 0.0 .. 1.0
	//eg. x * 0.5 + 0.5
	//cb->graphicsPipeline->minDepthBounds;
	//Clipper Z Scale and Offset
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_Z_SCALE_AND_OFFSET_length);
	clInsertClipperZScaleOffset(&commandBuffer->binCl, 0.5f, 0.5f);

	//Viewport Offset
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VIEWPORT_OFFSET_length);
	clInsertViewPortOffset(&commandBuffer->binCl, i->width >> 1, i->height >> 1);

	//TODO?
	//Flat Shade Flags
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_FLAT_SHADE_FLAGS_length);
	clInsertFlatShadeFlags(&commandBuffer->binCl, 0);

	//TODO how to get address?
	//GL Shader State
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_GL_SHADER_STATE_length);
	clInsertShaderState(&commandBuffer->binCl, 0, 0, cb->graphicsPipeline->vertexAttributeDescriptionCount);

	//Vertex Array Primitives (draw call)
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VERTEX_ARRAY_PRIMITIVES_length);
	clInsertVertexArrayPrimitives(&commandBuffer->binCl, firstVertex, vertexCount, getPrimitiveMode(cb->graphicsPipeline->topology));

	//emit shader record
	ControlListAddress fragCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]))->bos[VK_RPI_ASSEMBLY_TYPE_FRAGMENT],
		.offset = 0,
	};

	ControlListAddress vertCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]))->bos[VK_RPI_ASSEMBLY_TYPE_VERTEX],
		.offset = 0,
	};

	ControlListAddress coordCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]))->bos[VK_RPI_ASSEMBLY_TYPE_COORDINATE],
		.offset = 0,
	};

	//TODO
	commandBuffer->shaderRecCount++;
	clFit(commandBuffer, &commandBuffer->shaderRecCl, V3D21_SHADER_RECORD_length);
	ControlList relocCl = commandBuffer->shaderRecCl;
	//TODO number of attribs
	//3 is the number of type of possible shaders
	int numAttribs = 1;
	for(int c = 0; c < (3 + numAttribs)*4; ++c)
	{
		clInsertNop(&commandBuffer->shaderRecCl);
	}
	clInsertShaderRecord(&commandBuffer->shaderRecCl,
						 &relocCl,
						 &commandBuffer->handlesCl,
						 1, //TODO single threaded?
						 0, //point size included in shaded vertex data?
						 1, //enable clipping?
						 0, //fragment number of unused uniforms?
						 0, //fragment number of varyings?
						 0, //fragment uniform address?
						 fragCode, //fragment code address
						 0, //vertex number of unused uniforms?
						 1, //TODO vertex attribute array select bits
						 8, //TODO vertex total attribute size
						 0, //vertex uniform address
						 vertCode, //vertex shader code address
						 0, //coordinate number of unused uniforms?
						 1, //TODO coordinate attribute array select bits
						 8, //TODO coordinate total attribute size
						 0, //coordinate uniform address
						 coordCode  //coordinate shader code address
						 );

	ControlListAddress vertexBuffer = {
		.handle = cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[0].location]->boundMem->bo,
		.offset = 0,
	};

	clFit(commandBuffer, &commandBuffer->shaderRecCl, V3D21_ATTRIBUTE_RECORD_length);
	clInsertAttributeRecord(&commandBuffer->shaderRecCl,
							&relocCl,
							&commandBuffer->handlesCl,
							vertexBuffer, //address
							getFormatByteSize(cb->graphicsPipeline->vertexAttributeDescriptions[0].format),
							cb->graphicsPipeline->vertexBindingDescriptions[0].stride, //stride
							0, //TODO vertex vpm offset
							0  //TODO coordinte vpm offset
							);

	//insert vertex buffer handle
	//clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	//uint32_t vboIdx = clGetHandleIndex(&commandBuffer->handlesCl, vertexBuffer.handle);

	//insert shader code handles
	//clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	//uint32_t vertIdx = clGetHandleIndex(&commandBuffer->handlesCl, vertCode.handle);
	//clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	//uint32_t coordIdx = clGetHandleIndex(&commandBuffer->handlesCl, coordCode.handle);
	//clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	//uint32_t fragIdx = clGetHandleIndex(&commandBuffer->handlesCl, fragCode.handle);

	//Insert image handle index
	clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	uint32_t imageIdx = clGetHandleIndex(&commandBuffer->handlesCl, i->handle);

	//fill out submit cl fields
	commandBuffer->submitCl.color_write.hindex = imageIdx;
	commandBuffer->submitCl.color_write.offset = 0;
	commandBuffer->submitCl.color_write.flags = 0;
	//TODO format
	commandBuffer->submitCl.color_write.bits =
			VC4_SET_FIELD(VC4_RENDER_CONFIG_FORMAT_RGBA8888, VC4_RENDER_CONFIG_FORMAT) |
			VC4_SET_FIELD(i->tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);

	commandBuffer->submitCl.clear_color[0] = i->clearColor[0];
	commandBuffer->submitCl.clear_color[1] = i->clearColor[1];

	commandBuffer->submitCl.min_x_tile = 0;
	commandBuffer->submitCl.min_y_tile = 0;

	uint32_t tileSizeW = 64;
	uint32_t tileSizeH = 64;

	if(i->samples > 1)
	{
		tileSizeW >>= 1;
		tileSizeH >>= 1;
	}

	if(getFormatBpp(i->format) == 64)
	{
		tileSizeH >>= 1;
	}

	uint32_t widthInTiles = divRoundUp(i->width, tileSizeW);
	uint32_t heightInTiles = divRoundUp(i->height, tileSizeH);

	commandBuffer->submitCl.max_x_tile = widthInTiles - 1;
	commandBuffer->submitCl.max_y_tile = heightInTiles - 1;
	commandBuffer->submitCl.width = i->width;
	commandBuffer->submitCl.height = i->height;
	commandBuffer->submitCl.flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;
	commandBuffer->submitCl.clear_z = 0; //TODO
	commandBuffer->submitCl.clear_s = 0;

	//write uniforms
	//TODO
	/**
	//FS
	uniform count : 1
	tex sample count : 0
	uniform constant : 4291579008

	//VS
	uniform count : 4
	tex sample count : 0
	uniform constant : 1065353216
	uniform viewport xscale : 15360.000000
	uniform viewport yscale : -8640.000000
	uniform viewport zoffset : 0.500000

	//CS (same as VS)
	uniform count : 4
	tex sample count : 0
	uniform viewport yscale : -8640.000000
	uniform constant : 1065353216
	uniform viewport xscale : 15360.000000
	uniform viewport zoffset : 0.500000
	/**/
	clFit(commandBuffer, &commandBuffer->uniformsCl, 4*(1+4+4));
	//FS
	clInsertUniformConstant(&commandBuffer->uniformsCl, 4291579008);
	//VS
	clInsertUniformConstant(&commandBuffer->uniformsCl, 1065353216);
	clInsertUniformXYScale(&commandBuffer->uniformsCl, (float)(i->width) * 0.5f * 16.0f);
	clInsertUniformXYScale(&commandBuffer->uniformsCl, -1.0f * (float)(i->height) * 0.5f * 16.0f);
	clInsertUniformZOffset(&commandBuffer->uniformsCl, 0.5f);
	//CS
	clInsertUniformXYScale(&commandBuffer->uniformsCl, -1.0f * (float)(i->height) * 0.5f * 16.0f);
	clInsertUniformConstant(&commandBuffer->uniformsCl, 1065353216);
	clInsertUniformXYScale(&commandBuffer->uniformsCl, (float)(i->width) * 0.5f * 16.0f);
	clInsertUniformZOffset(&commandBuffer->uniformsCl, 0.5f);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdEndRenderPass
 */
void vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
	assert(commandBuffer);

	//TODO switch command buffer to next control record stream?
	//Ending a render pass instance performs any multisample resolve operations on the final subpass
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateRenderPass
 */
VkResult vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
	assert(device);
	assert(pCreateInfo);
	assert(pRenderPass);

	assert(pAllocator == 0); //TODO allocators not supported yet

	//just copy all data from create info
	//we'll later need to bake the control list based on this

	_renderpass* rp = malloc(sizeof(_renderpass));
	if(!rp)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	rp->numAttachments = pCreateInfo->attachmentCount;
	rp->attachments = malloc(sizeof(VkAttachmentDescription)*rp->numAttachments);
	if(!rp->attachments)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(rp->attachments, pCreateInfo->pAttachments, sizeof(VkAttachmentDescription)*rp->numAttachments);

	rp->numSubpasses = pCreateInfo->subpassCount;
	rp->subpasses = malloc(sizeof(VkSubpassDescription)*rp->numSubpasses);
	if(!rp->subpasses)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(int c = 0; c < rp->numSubpasses; ++c)
	{
		rp->subpasses[c].flags = pCreateInfo->pSubpasses[c].flags;
		rp->subpasses[c].pipelineBindPoint = pCreateInfo->pSubpasses[c].pipelineBindPoint;
		rp->subpasses[c].inputAttachmentCount = pCreateInfo->pSubpasses[c].inputAttachmentCount;
		rp->subpasses[c].colorAttachmentCount = pCreateInfo->pSubpasses[c].colorAttachmentCount;
		rp->subpasses[c].preserveAttachmentCount = pCreateInfo->pSubpasses[c].preserveAttachmentCount;

		if(rp->subpasses[c].inputAttachmentCount)
		{
			rp->subpasses[c].pInputAttachments = malloc(sizeof(VkAttachmentReference)*rp->subpasses[c].inputAttachmentCount);
			if(!rp->subpasses[c].pInputAttachments)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(rp->subpasses[c].pInputAttachments, pCreateInfo->pSubpasses[c].pInputAttachments, sizeof(VkAttachmentReference)*rp->subpasses[c].inputAttachmentCount);
		}
		else
		{
			rp->subpasses[c].pInputAttachments = 0;
		}

		if(rp->subpasses[c].colorAttachmentCount)
		{
			rp->subpasses[c].pColorAttachments = malloc(sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount);
			if(!rp->subpasses[c].pColorAttachments)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(rp->subpasses[c].pColorAttachments, pCreateInfo->pSubpasses[c].pColorAttachments, sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount);
		}
		else
		{
			rp->subpasses[c].pColorAttachments = 0;
		}

		if(rp->subpasses[c].colorAttachmentCount && pCreateInfo->pSubpasses[c].pResolveAttachments)
		{
			rp->subpasses[c].pResolveAttachments = malloc(sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount);
			if(!rp->subpasses[c].pResolveAttachments)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(rp->subpasses[c].pResolveAttachments, pCreateInfo->pSubpasses[c].pResolveAttachments, sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount);
		}
		else
		{
			rp->subpasses[c].pResolveAttachments = 0;
		}

		if(pCreateInfo->pSubpasses[c].pDepthStencilAttachment)
		{
			rp->subpasses[c].pDepthStencilAttachment = malloc(sizeof(VkAttachmentReference));
			if(!rp->subpasses[c].pDepthStencilAttachment)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(rp->subpasses[c].pDepthStencilAttachment, pCreateInfo->pSubpasses[c].pDepthStencilAttachment, sizeof(VkAttachmentReference));
		}
		else
		{
			rp->subpasses[c].pDepthStencilAttachment = 0;
		}

		if(rp->subpasses[c].preserveAttachmentCount)
		{
			rp->subpasses[c].pPreserveAttachments = malloc(sizeof(uint32_t)*rp->subpasses[c].preserveAttachmentCount);
			if(!rp->subpasses[c].pPreserveAttachments)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(rp->subpasses[c].pPreserveAttachments, pCreateInfo->pSubpasses[c].pPreserveAttachments, sizeof(uint32_t)*rp->subpasses[c].preserveAttachmentCount);
		}
		else
		{
			rp->subpasses[c].pPreserveAttachments = 0;
		}
	}

	rp->numSubpassDependencies = pCreateInfo->dependencyCount;
	rp->subpassDependencies = malloc(sizeof(VkSubpassDependency)*rp->numSubpassDependencies);
	if(!rp->subpassDependencies)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(rp->subpassDependencies, pCreateInfo->pDependencies, sizeof(VkSubpassDependency)*rp->numSubpassDependencies);

	*pRenderPass = rp;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImageView
 */
VkResult vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	assert(pAllocator == 0); //TODO

	_imageView* view = malloc(sizeof(_imageView));

	if(!view)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	view->image = pCreateInfo->image;
	view->viewType = pCreateInfo->viewType;
	view->interpretedFormat = pCreateInfo->format;
	view->swizzle = pCreateInfo->components;
	view->subresourceRange = pCreateInfo->subresourceRange;

	//TODO errors/validation

	*pView = view;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateFramebuffer
 */
VkResult vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pFramebuffer);

	assert(pAllocator == 0); //TODO

	_framebuffer* fb = malloc(sizeof(_framebuffer));

	if(!fb)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	fb->renderpass = pCreateInfo->renderPass;

	fb->numAttachmentViews = pCreateInfo->attachmentCount;
	fb->attachmentViews = malloc(sizeof(_imageView) * fb->numAttachmentViews);

	if(!fb->attachmentViews)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(int c = 0; c < fb->numAttachmentViews; ++c)
	{
		memcpy(&fb->attachmentViews[c], pCreateInfo->pAttachments[c], sizeof(_imageView));
	}

	fb->width = pCreateInfo->width;
	fb->height = pCreateInfo->height;
	fb->layers = pCreateInfo->layers;

	//TODO errors/validation

	*pFramebuffer = fb;

	return VK_SUCCESS;
}


VkResult vkCreateShaderModuleFromRpiAssemblyKHR(VkDevice device, VkRpiShaderModuleAssemblyCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	assert(device);
	assert(pCreateInfo);
	assert(pShaderModule);
	assert(pCreateInfo->byteStreamArray);
	assert(pCreateInfo->numBytesArray);

	assert(pAllocator == 0); //TODO

	_shaderModule* shader = malloc(sizeof(_shaderModule));

	if(!shader)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(int c = 0; c < VK_RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(pCreateInfo->byteStreamArray[c])
		{
			uint32_t size = pCreateInfo->numBytesArray[c];
			shader->bos[c] = vc4_bo_alloc_shader(controlFd, pCreateInfo->byteStreamArray[c], &size);
			shader->sizes[c] = size;
		}
		else
		{
			shader->bos[c] = 0;
		}
	}

	*pShaderModule = shader;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateShaderModule
 */
VkResult vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateGraphicsPipelines
 */
VkResult vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	assert(device);
	assert(createInfoCount > 0);
	assert(pCreateInfos);
	assert(pPipelines);

	assert(pipelineCache == 0); //TODO not supported right now
	assert(pAllocator == 0); //TODO

	for(int c = 0; c < createInfoCount; ++c)
	{
		_pipeline* pip = malloc(sizeof(_pipeline));
		if(!pip)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		for(int d = 0; d < pCreateInfos->stageCount; ++d)
		{
			uint32_t idx = ulog2(pCreateInfos->pStages[d].stage);
			pip->modules[idx] = pCreateInfos->pStages[d].module;

			pip->names[idx] = malloc(strlen(pCreateInfos->pStages[d].pName)+1);
			if(!pip->names[idx])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(pip->names[idx], pCreateInfos->pStages[d].pName, strlen(pCreateInfos->pStages[d].pName)+1);
		}

		pip->vertexAttributeDescriptionCount = pCreateInfos->pVertexInputState->vertexAttributeDescriptionCount;
		pip->vertexAttributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount);
		if(!pip->vertexAttributeDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexAttributeDescriptions, pCreateInfos->pVertexInputState->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount);

		pip->vertexBindingDescriptionCount = pCreateInfos->pVertexInputState->vertexBindingDescriptionCount;
		pip->vertexBindingDescriptions = malloc(sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount);
		if(!pip->vertexBindingDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexBindingDescriptions, pCreateInfos->pVertexInputState->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount);

		pip->topology = pCreateInfos->pInputAssemblyState->topology;
		pip->primitiveRestartEnable = pCreateInfos->pInputAssemblyState->primitiveRestartEnable;

		//TODO tessellation ignored

		pip->viewportCount = pCreateInfos->pViewportState->viewportCount;
		pip->viewports = malloc(sizeof(VkViewport) * pip->viewportCount);
		if(!pip->viewports)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->viewports, pCreateInfos->pViewportState->pViewports, sizeof(VkViewport) * pip->viewportCount);


		pip->scissorCount = pCreateInfos->pViewportState->scissorCount;
		pip->scissors = malloc(sizeof(VkRect2D) * pip->viewportCount);
		if(!pip->scissors)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->scissors, pCreateInfos->pViewportState->pScissors, sizeof(VkRect2D) * pip->scissorCount);

		pip->depthClampEnable = pCreateInfos->pRasterizationState->depthClampEnable;
		pip->rasterizerDiscardEnable = pCreateInfos->pRasterizationState->rasterizerDiscardEnable;
		pip->polygonMode = pCreateInfos->pRasterizationState->polygonMode;
		pip->cullMode = pCreateInfos->pRasterizationState->cullMode;
		pip->frontFace = pCreateInfos->pRasterizationState->frontFace;
		pip->depthBiasEnable = pCreateInfos->pRasterizationState->depthBiasEnable;
		pip->depthBiasConstantFactor = pCreateInfos->pRasterizationState->depthBiasConstantFactor;
		pip->depthBiasClamp = pCreateInfos->pRasterizationState->depthBiasClamp;
		pip->depthBiasSlopeFactor = pCreateInfos->pRasterizationState->depthBiasSlopeFactor;
		pip->lineWidth = pCreateInfos->pRasterizationState->lineWidth;

		pip->rasterizationSamples = pCreateInfos->pMultisampleState->rasterizationSamples;
		pip->sampleShadingEnable = pCreateInfos->pMultisampleState->sampleShadingEnable;
		pip->minSampleShading = pCreateInfos->pMultisampleState->minSampleShading;
		if(pCreateInfos->pMultisampleState->pSampleMask)
		{
			pip->sampleMask = *pCreateInfos->pMultisampleState->pSampleMask;
		}
		else
		{
			pip->sampleMask = 0;
		}
		pip->alphaToCoverageEnable = pCreateInfos->pMultisampleState->alphaToCoverageEnable;
		pip->alphaToOneEnable = pCreateInfos->pMultisampleState->alphaToOneEnable;

		pip->depthTestEnable = pCreateInfos->pDepthStencilState->depthTestEnable;
		pip->depthWriteEnable = pCreateInfos->pDepthStencilState->depthWriteEnable;
		pip->depthCompareOp = pCreateInfos->pDepthStencilState->depthCompareOp;
		pip->depthBoundsTestEnable = pCreateInfos->pDepthStencilState->depthBoundsTestEnable;
		pip->stencilTestEnable = pCreateInfos->pDepthStencilState->stencilTestEnable;
		pip->front = pCreateInfos->pDepthStencilState->front;
		pip->back = pCreateInfos->pDepthStencilState->back;
		pip->minDepthBounds = pCreateInfos->pDepthStencilState->minDepthBounds;
		pip->maxDepthBounds = pCreateInfos->pDepthStencilState->maxDepthBounds;

		pip->logicOpEnable = pCreateInfos->pColorBlendState->logicOpEnable;
		pip->logicOp = pCreateInfos->pColorBlendState->logicOp;
		pip->attachmentCount = pCreateInfos->pColorBlendState->attachmentCount;
		pip->attachmentBlendStates = malloc(sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount);
		if(!pip->attachmentBlendStates)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->attachmentBlendStates, pCreateInfos->pColorBlendState->pAttachments, sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount);

		memcpy(pip->blendConstants, pCreateInfos->pColorBlendState, sizeof(float)*4);


		if(pCreateInfos->pDynamicState)
		{
			pip->dynamicStateCount = pCreateInfos->pDynamicState->dynamicStateCount;
			pip->dynamicStates = malloc(sizeof(VkDynamicState)*pip->dynamicStateCount);
			if(!pip->dynamicStates)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(pip->dynamicStates, pCreateInfos->pDynamicState->pDynamicStates, sizeof(VkDynamicState)*pip->dynamicStateCount);
		}
		else
		{
			pip->dynamicStateCount = 0;
			pip->dynamicStates = 0;
		}

		pip->layout = pCreateInfos->layout;
		pip->renderPass = pCreateInfos->renderPass;
		pip->subpass = pCreateInfos->subpass;

		//TODO derivative pipelines ignored

		pPipelines[c] = pip;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceMemoryProperties
 */
void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
	assert(physicalDevice);
	assert(pMemoryProperties);

	if(memoryHeaps[0].size == 0)
	{
		//TODO is this the correct way of getting amount of video mem?
		char buf[4096];
		int fd = open("/proc/meminfo", O_RDONLY);
		read(fd, buf, 4096);
		close(fd);
		char* cma = strstr(buf, "CmaTotal");
		char* cmaend = strstr(cma, "\n");
		char cmaAmount[4096];
		char* cmaPtr = cmaAmount;
		while(cma != cmaend)
		{
			if(*cma >= '0' && *cma <= '9')
			{
				//number
				*cmaPtr = *cma; //copy char
				cmaPtr++;
			}

			cma++;
		}
		*cmaPtr = '\0';
		unsigned amount = atoi(cmaAmount);
		//printf("%i\n", amount);

		//all heaps share the same memory
		for(int c = 0; c < numMemoryHeaps; ++c)
		{
			memoryHeaps[c].size = amount;
		}
	}

	pMemoryProperties->memoryTypeCount = numMemoryTypes;
	for(int c = 0; c < numMemoryTypes; ++c)
	{
		pMemoryProperties->memoryTypes[c] = memoryTypes[c];
	}

	pMemoryProperties->memoryHeapCount = numMemoryHeaps;
	for(int c = 0; c < numMemoryHeaps; ++c)
	{
		pMemoryProperties->memoryHeaps[c] = memoryHeaps[c];
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindVertexBuffers
 */
void vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	for(int c = 0; c < bindingCount; ++c)
	{
		cb->vertexBuffers[firstBinding + c] = pBuffers[c];
		cb->vertexBufferOffsets[firstBinding + c] = pOffsets[c];
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBuffer
 */
VkResult vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pBuffer);

	assert(pAllocator == 0); //TODO

	_buffer* buf = malloc(sizeof(_buffer));
	if(!buf)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	buf->size = pCreateInfo->size;
	buf->usage = pCreateInfo->usage;
	buf->boundMem = 0;
	buf->alignment = ARM_PAGE_SIZE; //TODO
	buf->alignedSize = getBOAlignedSize(buf->size);

	*pBuffer = buf;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetBufferMemoryRequirements
 */
void vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	assert(device);
	assert(buffer);
	assert(pMemoryRequirements);

	pMemoryRequirements->alignment = ((_buffer*)buffer)->alignment;
	pMemoryRequirements->size = ((_buffer*)buffer)->alignedSize;
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; //TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAllocateMemory
 */
VkResult vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
	assert(device);
	assert(pAllocateInfo);
	assert(pMemory);

	assert(pAllocator == 0); //TODO

	uint32_t bo = vc4_bo_alloc(controlFd, pAllocateInfo->allocationSize, "vkAllocateMemory");
	if(!bo)
	{
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	_deviceMemory* mem = malloc(sizeof(_deviceMemory));
	if(!mem)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	mem->bo = bo;
	mem->size = pAllocateInfo->allocationSize;
	mem->memTypeIndex = pAllocateInfo->memoryTypeIndex;
	mem->mappedPtr = 0;

	*pMemory = mem;

	//TODO max number of allocations

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkMapMemory
 */
VkResult vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
	assert(device);
	assert(memory);
	assert(size);
	assert(ppData);

	assert(memoryTypes[((_deviceMemory*)memory)->memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	assert(!((_deviceMemory*)memory)->mappedPtr);
	assert(offset < ((_deviceMemory*)memory)->size);
	if(size != VK_WHOLE_SIZE)
	{
		assert(size > 0);
		assert(size <= ((_deviceMemory*)memory)->size - offset);
	}

	//TODO check ppdata alignment
	//TODO multiple instances?

	void* ptr = vc4_bo_map(controlFd, ((_deviceMemory*)memory)->bo, offset, size);
	if(!ptr)
	{
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	((_deviceMemory*)memory)->mappedPtr = ptr;
	((_deviceMemory*)memory)->mappedOffset = offset;
	((_deviceMemory*)memory)->mappedSize = size;
	*ppData = ptr;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkUnmapMemory
 */
void vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
	assert(device);
	assert(memory);

	vc4_bo_unmap_unsynchronized(controlFd, ((_deviceMemory*)memory)->mappedPtr, ((_deviceMemory*)memory)->mappedSize);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindBufferMemory
 */
VkResult vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	assert(device);
	assert(buffer);
	assert(memory);

	_buffer* buf = buffer;
	_deviceMemory* mem = memory;

	assert(!buf->boundMem);
	assert(memoryOffset < mem->size);
	assert(memoryOffset % buf->alignment == 0);
	assert(buf->alignedSize <= mem->size - memoryOffset);

	buf->boundMem = mem;
	buf->boundOffset = memoryOffset;

	return VK_SUCCESS;
}






void vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(buffer);

	assert(pAllocator == 0); //TODO

	_buffer* buf = buffer;
	free(buf);
}

void vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(memory);

	assert(pAllocator == 0); //TODO

	_deviceMemory* mem = memory;
	vc4_bo_free(controlFd, mem->bo, mem->mappedPtr, mem->size);
	free(mem);
}

void vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
	//TODO
}

void vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(imageView);

	assert(pAllocator == 0); //TODO

	_imageView* view = imageView;
	free(view);
}

void vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(framebuffer);

	assert(pAllocator == 0); //TODO

	_framebuffer* fb = framebuffer;
	free(fb->attachmentViews);
	free(fb);
}

void vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(renderPass);

	assert(pAllocator == 0); //TODO

	_renderpass* rp = renderPass;

	free(rp->subpassDependencies);

	for(int c = 0; c < rp->numSubpasses; ++c)
	{
		free(rp->subpasses[c].pInputAttachments);
		free(rp->subpasses[c].pColorAttachments);
		free(rp->subpasses[c].pResolveAttachments);
		free(rp->subpasses[c].pDepthStencilAttachment);
		free(rp->subpasses[c].pPreserveAttachments);
	}

	free(rp->subpasses);

	free(rp->attachments);

	free(rp);
}

void vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(shaderModule);

	assert(pAllocator == 0);

	_shaderModule* shader = shaderModule;

	for(int c = 0; c < VK_RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(shader->bos[c])
		{
			vc4_bo_free(controlFd, shader->bos[c], 0, shader->sizes[c]);
		}
	}

	free(shader);
}

void vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(pipeline);

	assert(pAllocator == 0); //TODO

	_pipeline* pip = pipeline;

	free(pip->dynamicStates);
	free(pip->attachmentBlendStates);
	free(pip->scissors);
	free(pip->viewports);
	free(pip->vertexBindingDescriptions);
	free(pip->vertexAttributeDescriptions);

	for(int c = 0; c < 6; ++c)
	{
		free(pip->names[c]);
	}

	free(pip);
}
