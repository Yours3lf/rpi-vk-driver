#include "common.h"

#include "command.h"

#include "kernel/vc4_packet.h"

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
