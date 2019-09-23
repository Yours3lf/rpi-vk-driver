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

	//TODO make this as lightweight as possible to make sure
	//as many drawcalls can be submitted as possible

	//uint32_t vertexBufferDirty;
	//uint32_t indexBufferDirty;
	///uint32_t viewportDirty;
	///uint32_t lineWidthDirty;
	///uint32_t depthBiasDirty;
	///uint32_t depthBoundsDirty;
	//uint32_t graphicsPipelineDirty;
	//uint32_t computePipelineDirty;
	//uint32_t subpassDirty;
	//uint32_t blendConstantsDirty;
	//uint32_t scissorDirty;
	//uint32_t stencilCompareMaskDirty;
	//uint32_t stencilWriteMaskDirty;
	//uint32_t stencilReferenceDirty;
	//uint32_t descriptorSetDirty;
	//uint32_t pushConstantDirty;

	//TODO multiple viewports
	VkViewport vp;
	vp = cb->graphicsPipeline->viewports[0];

	for(uint32_t c = 0; c < cb->graphicsPipeline->dynamicStateCount; ++c)
	{
		if(cb->graphicsPipeline->dynamicStates[c] == VK_DYNAMIC_STATE_VIEWPORT)
		{
			vp = cb->viewport;
		}
	}

	//if(cb->lineWidthDirty)
	{
		//Line width
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_LINE_WIDTH_length);
		clInsertLineWidth(&commandBuffer->binCl, cb->graphicsPipeline->lineWidth);

		cb->lineWidthDirty = 0;
	}

	//if(cb->viewportDirty)
	{
		//Clip Window
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIP_WINDOW_length);
		clInsertClipWindow(&commandBuffer->binCl,
						   vp.width,
						   vp.height,
						   vp.y, //bottom pixel coord
						   vp.x); //left pixel coord

		//TODO why flipped???
		//Clipper XY Scaling
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_XY_SCALING_length);
		clInsertClipperXYScaling(&commandBuffer->binCl, (float)(vp.width) * 0.5f * 16.0f, -1.0f * (float)(vp.height) * 0.5f * 16.0f);

		//Viewport Offset
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_VIEWPORT_OFFSET_length);
		clInsertViewPortOffset(&commandBuffer->binCl, ((int16_t)vp.width) >> 1, ((int16_t)vp.height) >> 1);

		cb->viewportDirty = 0;
	}

	//if(cb->depthBiasDirty || cb->depthBoundsDirty)
	{
		//Configuration Bits
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CONFIGURATION_BITS_length);
		clInsertConfigurationBits(&commandBuffer->binCl,
								  1, //earlyz updates enable
								  1, //earlyz enable
								  cb->graphicsPipeline->depthWriteEnable, //z updates enable
								  cb->graphicsPipeline->depthTestEnable ? getCompareOp(cb->graphicsPipeline->depthCompareOp) : V3D_COMPARE_FUNC_ALWAYS, //depth compare func
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

		//TODO how is this calculated?
		//it's Zc to Zs scale and bias
		//seems to go from -1.0 .. 1.0 to 0.0 .. 1.0
		//eg. x * 0.5 + 0.5
		//cb->graphicsPipeline->minDepthBounds;
		//Clipper Z Scale and Offset
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_Z_SCALE_AND_OFFSET_length);
		clInsertClipperZScaleOffset(&commandBuffer->binCl, 0.5f, 0.5f);

		cb->vertexBufferDirty = 0;
		cb->depthBoundsDirty = 0;
	}

	//Point size
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_POINT_SIZE_length);
	clInsertPointSize(&commandBuffer->binCl, 1.0f);

	//TODO?
	//Flat Shade Flags
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_FLAT_SHADE_FLAGS_length);
	clInsertFlatShadeFlags(&commandBuffer->binCl, 0);









	//GL Shader State
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_GL_SHADER_STATE_length);
	clInsertShaderState(&commandBuffer->binCl,
						0, //shader state record address
						0, //extended shader state record
						cb->graphicsPipeline->vertexAttributeDescriptionCount & 0x7); //number of attribute arrays, 0 -> 8

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

	uint32_t attribCount = 0;
	uint32_t attribSelectBits = 0;
	for(uint32_t c = 0 ; c < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++c)
	{
		if(cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding])
		{
			attribCount++;
			attribSelectBits |= 1 << cb->graphicsPipeline->vertexAttributeDescriptions[c].location;
		}
	}

	uint32_t attribSize = 0;
	for(uint32_t c = 0; c < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++c)
	{
		attribSize += getFormatByteSize(cb->graphicsPipeline->vertexAttributeDescriptions[c].format);
	}

	//TODO number of attribs
	//3 is the number of type of possible shaders
	for(int c = 0; c < (3 + attribCount)*4; ++c)
	{
		clInsertNop(&commandBuffer->shaderRecCl);
	}
	clInsertShaderRecord(&commandBuffer->shaderRecCl,
						 &relocCl,
						 &commandBuffer->handlesCl,
						 cb->binCl.currMarker->handlesBuf,
						 cb->binCl.currMarker->handlesSize,
						 !cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->hasThreadSwitch,
						 0, //TODO point size included in shaded vertex data?
						 1, //enable clipping
						 0, //TODO fragment number of used uniforms?
						 cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)]->numVaryings, //fragment number of varyings
						 0, //fragment uniform address?
						 fragCode, //fragment code address
						 0, //TODO vertex number of used uniforms?
						 attribSelectBits, //vertex attribute array select bits
						 attribSize, //vertex total attribute size
						 0, //vertex uniform address
						 vertCode, //vertex shader code address
						 0, //TODO coordinate number of used uniforms?
						 //TODO how do we know which attribute contains the vertices?
						 //for now the first one will be hardcoded to have the vertices...
						 1 << 0, //coordinate attribute array select bits
						 getFormatByteSize(cb->graphicsPipeline->vertexAttributeDescriptions[0].format), //coordinate total attribute size
						 0, //coordinate uniform address
						 coordCode  //coordinate shader code address
						 );

	for(uint32_t c = 0 ; c < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++c)
	{
		if(cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding])
		{
			ControlListAddress vertexBuffer = {
				.handle = cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundMem->bo,
				.offset = cb->graphicsPipeline->vertexAttributeDescriptions[c].offset,
			};

			clFit(commandBuffer, &commandBuffer->shaderRecCl, V3D21_ATTRIBUTE_RECORD_length);
			clInsertAttributeRecord(&commandBuffer->shaderRecCl,
									&relocCl,
									&commandBuffer->handlesCl,
									cb->binCl.currMarker->handlesBuf,
									cb->binCl.currMarker->handlesSize,
									vertexBuffer, //reloc address
									getFormatByteSize(cb->graphicsPipeline->vertexAttributeDescriptions[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding].format),
									cb->graphicsPipeline->vertexBindingDescriptions[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding].stride, //stride
									cb->graphicsPipeline->vertexAttributeDescriptions[c].offset, //vertex vpm offset
									cb->graphicsPipeline->vertexAttributeDescriptions[c].offset  //coordinte vpm offset
									);
		}
	}







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
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, cb->binCl.currMarker->handlesBuf, cb->binCl.currMarker->handlesSize, di->imageView->image->boundMem->bo);

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
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, cb->binCl.currMarker->handlesBuf, cb->binCl.currMarker->handlesSize, db->buffer->boundMem->bo);

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
					uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, cb->binCl.currMarker->handlesBuf, cb->binCl.currMarker->handlesSize, dtb->bufferView->buffer->boundMem->bo);

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

	cb->numDrawCallsSubmitted++;
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
	UNSUPPORTED(vkCmdDrawIndexedIndirect);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{
	UNSUPPORTED(vkCmdDrawIndirect);
}
