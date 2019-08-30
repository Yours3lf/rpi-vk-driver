#include "common.h"

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

	//TODO handle cases when submitting >65k vertices in a VBO
	//TODO HW-2116 workaround
	//TODO GFXH-515 / SW-5891 workaround

	//TODO handle multiple attachments etc.
	_image* i = fb->attachmentViews[rp->subpasses[cb->currentSubpass].pColorAttachments[0].attachment].image;

	//TODO when doing multiple draw calls (well multipass now)
	//kernel side expects one Tile Binning Mode Config field per submit
	//sounds like for each renderpass we'll have to submit one submit, one of these config fields

	//stuff needed to submit a draw call:
	//Tile Binning Mode Configuration
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
	clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
										 0, //double buffer in non ms mode
										 0, //tile allocation block size
										 0, //tile allocation initial block size
										 0, //auto initialize tile state data array
										 getFormatBpp(i->format) == 64, //64 bit color mode
										 i->samples > 1, //msaa
										 i->width, i->height,
										 0, //tile state data array address
										 0, //tile allocation memory size
										 0); //tile allocation memory address

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
	clInsertClipWindow(&commandBuffer->binCl,
					   i->width,
					   i->height,
					   0, //bottom pixel coord
					   0); //left pixel coord

	//Configuration Bits
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_CONFIGURATION_BITS_length);
	clInsertConfigurationBits(&commandBuffer->binCl,
							  1, //TODO earlyz updates enable
							  0, //TODO earlyz enable
							  0, //TODO z updates enable
							  cb->graphicsPipeline->depthTestEnable ? getDepthCompareOp(cb->graphicsPipeline->depthCompareOp) : V3D_COMPARE_FUNC_ALWAYS, //depth compare func
							  0, //coverage read mode
							  0, //coverage pipe select
							  0, //coverage update mode
							  0, //coverage read type
							  0, //rasterizer oversample mode
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
	//it's Zc to Zs scale and bias
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
	clInsertShaderState(&commandBuffer->binCl,
						0, //shader state record address
						0, //extended shader state record
						cb->graphicsPipeline->vertexAttributeDescriptionCount);

	//Vertex Array Primitives (draw call)
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VERTEX_ARRAY_PRIMITIVES_length);
	clInsertVertexArrayPrimitives(&commandBuffer->binCl, firstVertex, vertexCount, getPrimitiveMode(cb->graphicsPipeline->topology));

	//emit shader record
	ControlListAddress fragCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]))->bos[RPI_ASSEMBLY_TYPE_FRAGMENT],
		.offset = 0,
	};

	ControlListAddress vertCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]))->bos[RPI_ASSEMBLY_TYPE_VERTEX],
		.offset = 0,
	};

	ControlListAddress coordCode = {
		.handle = ((_shaderModule*)(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]))->bos[RPI_ASSEMBLY_TYPE_COORDINATE],
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
						 !cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->hasThreadSwitch,
						 0, //TODO point size included in shaded vertex data?
						 1, //TODO enable clipping?
						 0, //TODO fragment number of unused uniforms?
						 0, //TODO fragment number of varyings?
						 0, //fragment uniform address?
						 fragCode, //fragment code address
						 0, //TODO vertex number of unused uniforms?
						 1, //TODO vertex attribute array select bits
						 8, //TODO vertex total attribute size
						 0, //vertex uniform address
						 vertCode, //vertex shader code address
						 0, //TODO coordinate number of unused uniforms?
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
							vertexBuffer, //reloc address
							getFormatByteSize(cb->graphicsPipeline->vertexAttributeDescriptions[0].format),
							cb->graphicsPipeline->vertexBindingDescriptions[0].stride, //stride
							0, //TODO vertex vpm offset
							0  //TODO coordinte vpm offset
							);

	//Insert image handle index
	clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	uint32_t imageIdx = clGetHandleIndex(&commandBuffer->handlesCl, i->boundMem->bo);

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
	_pipelineLayout* pl = cb->graphicsPipeline->layout;

	//kernel side expects relocations first!
	for(uint32_t c = 0; c < cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->numMappings; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->mappings[c];

		if(mapping.shaderStage & VK_SHADER_STAGE_FRAGMENT_BIT)
		{
			if(mapping.mappingType == VK_RPI_ASSEMBLY_MAPPING_TYPE_DESCRIPTOR)
			{
				if(mapping.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
				   mapping.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
				   mapping.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				{
					_descriptorSet* ds = getMapElement(pl->descriptorSetBindingMap, mapping.descriptorSet);
					_descriptorImage* di = getMapElement(ds->imageBindingMap, mapping.descriptorBinding);
					di += mapping.descriptorArrayElement;

					//emit reloc for texture BO
					clFit(commandBuffer, &commandBuffer->handlesCl, 4);
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, di->imageView->image->boundMem->bo);

					//emit tex bo reloc index
					clFit(commandBuffer, &commandBuffer->uniformsCl, 4);
					clInsertData(&commandBuffer->uniformsCl, 4, &idx);
				}
				else if(mapping.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
						mapping.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
						mapping.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
						mapping.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
				{
					_descriptorSet* ds = getMapElement(pl->descriptorSetBindingMap, mapping.descriptorSet);
					_descriptorBuffer* db = getMapElement(ds->bufferBindingMap, mapping.descriptorBinding);
					db += mapping.descriptorArrayElement;

					//emit reloc for BO
					clFit(commandBuffer, &commandBuffer->handlesCl, 4);
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, db->buffer->boundMem->bo);

					//emit bo reloc index
					clFit(commandBuffer, &commandBuffer->uniformsCl, 4);
					clInsertData(&commandBuffer->uniformsCl, 4, &idx);
				}
				else if(mapping.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
						mapping.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
				{
					_descriptorSet* ds = getMapElement(pl->descriptorSetBindingMap, mapping.descriptorSet);
					_descriptorTexelBuffer* dtb = getMapElement(ds->texelBufferBindingMap, mapping.descriptorBinding);
					dtb += mapping.descriptorArrayElement;

					//emit reloc for BO
					clFit(commandBuffer, &commandBuffer->handlesCl, 4);
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, dtb->bufferView->buffer->boundMem->bo);

					//emit bo reloc index
					clFit(commandBuffer, &commandBuffer->uniformsCl, 4);
					clInsertData(&commandBuffer->uniformsCl, 4, &idx);
				}
				else
				{
					assert(0); //shouldn't happen
				}
			}
		}
	}

	//after relocs we can proceed with the usual uniforms
	for(uint32_t c = 0; c < cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->numMappings; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->mappings[c];

		if(mapping.shaderStage & VK_SHADER_STAGE_FRAGMENT_BIT)
		{
			if(mapping.mappingType == VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT)
			{
				clFit(commandBuffer, &commandBuffer->uniformsCl, 4);
				clInsertData(&commandBuffer->uniformsCl, 4, cb->pushConstantBufferPixel + mapping.resourceOffset);
			}
			else if(mapping.mappingType == VK_RPI_ASSEMBLY_MAPPING_TYPE_DESCRIPTOR)
			{
				if(mapping.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
				   mapping.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
				   mapping.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				{
					_descriptorSet* ds = getMapElement(pl->descriptorSetBindingMap, mapping.descriptorSet);
					_descriptorImage* di = getMapElement(ds->imageBindingMap, mapping.descriptorBinding);
					di += mapping.descriptorArrayElement;

					//TODO handle miplevels according to subresource rage?
					uint32_t params[4];
					encodeTextureUniform(params,
										 di->imageView->image->miplevels - 1,
										 getTextureDataType(di->imageView->interpretedFormat),
										 di->imageView->viewType == VK_IMAGE_VIEW_TYPE_CUBE,
										 0, //TODO cubemap stride
										 0, //TODO texture base ptr
										 di->imageView->image->height & 2047,
										 di->imageView->image->width & 2047,
										 getMinFilterType(di->sampler->minFilter, di->sampler->mipmapMode, di->sampler->maxLod),
										 di->sampler->magFilter == VK_FILTER_NEAREST,
										 getWrapMode(di->sampler->addressModeU),
										 getWrapMode(di->sampler->addressModeV),
										 0 //TODO no auto LOD
										 );

					uint32_t size = 0;
					if(di->imageView->viewType == VK_IMAGE_VIEW_TYPE_1D)
					{
						size = 4;
					}
					else if(di->imageView->viewType == VK_IMAGE_VIEW_TYPE_2D)
					{
						size = 8;
					}
					else if(di->imageView->viewType == VK_IMAGE_VIEW_TYPE_CUBE)
					{
						size = 12;
					}
					else
					{
						assert(0); //unsupported
					}

					//emit tex parameters
					clFit(commandBuffer, &commandBuffer->uniformsCl, size);
					clInsertData(&commandBuffer->uniformsCl, size, params);
				}
			}
		}
	}

	//do it twice for vertex and then coordinate
	for(uint32_t d = 0; d < 2; ++d)
	{
		for(uint32_t c = 0; c < cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]->numMappings; ++c)
		{
			VkRpiAssemblyMappingEXT mapping = cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)]->mappings[c];

			if(mapping.shaderStage & VK_SHADER_STAGE_VERTEX_BIT)
			{
				if(mapping.mappingType == VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT)
				{
					clFit(commandBuffer, &commandBuffer->uniformsCl, 4);
					clInsertData(&commandBuffer->uniformsCl, 4, cb->pushConstantBufferVertex + mapping.resourceOffset);
				}
				else if(mapping.mappingType == VK_RPI_ASSEMBLY_MAPPING_TYPE_DESCRIPTOR)
				{

				}
				else
				{
					assert(0); //shouldn't happen
				}
			}
		}
	}
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    indexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstIndex,
	int32_t                                     vertexOffset,
	uint32_t                                    firstInstance)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{

}
