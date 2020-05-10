#include "common.h"

#include "kernel/vc4_packet.h"

//returns max index
static uint32_t drawCommon(VkCommandBuffer commandBuffer, int32_t vertexOffset)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

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
						   vp.width - vp.x,
						   vp.height - vp.y,
						   vp.y, //bottom pixel coord
						   vp.x); //left pixel coord

		//Vulkan conventions, Y flipped [1...-1] bottom->top
		//Clipper XY Scaling
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_XY_SCALING_length);
		clInsertClipperXYScaling(&commandBuffer->binCl, (float)(vp.width) * 0.5f * 16.0f, 1.0f * (float)(vp.height) * 0.5f * 16.0f);

		//Viewport Offset
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_VIEWPORT_OFFSET_length);
		clInsertViewPortOffset(&commandBuffer->binCl, vp.width * 0.5f + vp.x, vp.height * 0.5f - vp.y);

		cb->viewportDirty = 0;
	}

	//if(cb->depthBiasDirty || cb->depthBoundsDirty)
	{
		//Configuration Bits
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CONFIGURATION_BITS_length);
		clInsertConfigurationBits(&commandBuffer->binCl,
								  1, //earlyz updates enable
								  cb->graphicsPipeline->depthTestEnable, //earlyz enable
								  cb->graphicsPipeline->depthWriteEnable && cb->graphicsPipeline->depthTestEnable, //z updates enable
								  cb->graphicsPipeline->depthTestEnable ? getCompareOp(cb->graphicsPipeline->depthCompareOp) : V3D_COMPARE_FUNC_ALWAYS, //depth compare func
								  0, //coverage read mode
								  0, //coverage pipe select
								  0, //coverage update mode
								  0, //coverage read type
								  cb->graphicsPipeline->rasterizationSamples > 1, //rasterizer oversample mode
								  cb->graphicsPipeline->depthBiasEnable, //depth offset enable
								  cb->graphicsPipeline->frontFace == VK_FRONT_FACE_CLOCKWISE, //clockwise
								  !(cb->graphicsPipeline->cullMode & VK_CULL_MODE_BACK_BIT), //enable back facing primitives
								  !(cb->graphicsPipeline->cullMode & VK_CULL_MODE_FRONT_BIT)); //enable front facing primitives

		//TODO Depth Offset
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_DEPTH_OFFSET_length);
		clInsertDepthOffset(&commandBuffer->binCl, cb->graphicsPipeline->depthBiasConstantFactor, cb->graphicsPipeline->depthBiasSlopeFactor);

		//Vulkan conventions, we expect the resulting NDC space Z axis to be in range [0...1] close->far
		//cb->graphicsPipeline->minDepthBounds;
		//Clipper Z Scale and Offset
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_CLIPPER_Z_SCALE_AND_OFFSET_length);
		clInsertClipperZScaleOffset(&commandBuffer->binCl, 0.0f, 1.0f);

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

	_shaderModule* vertModule = 0, *fragModule = 0;

	//it could be that all stages are contained in a single module, or have separate modules

	if(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)])
	{
		fragModule = cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_FRAGMENT_BIT)];
	}

	if(cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)])
	{
		vertModule = cb->graphicsPipeline->modules[ulog2(VK_SHADER_STAGE_VERTEX_BIT)];
	}

//	fprintf(stderr, "==============\n", fragModule);
//	fprintf(stderr, "fragModule %p\n", fragModule);
//	fprintf(stderr, "vertModule %p\n", vertModule);

	if(!vertModule)
	{
		vertModule = fragModule;
	}

	if(!fragModule)
	{
		fragModule = vertModule;
	}

//	fprintf(stderr, "fragModule %p\n", fragModule);
//	fprintf(stderr, "vertModule %p\n", vertModule);

	assert(fragModule);
	assert(vertModule);
	assert(fragModule->bos[VK_RPI_ASSEMBLY_TYPE_FRAGMENT]);
	assert(vertModule->bos[VK_RPI_ASSEMBLY_TYPE_VERTEX]);
	assert(vertModule->bos[VK_RPI_ASSEMBLY_TYPE_COORDINATE]);

	//emit shader record
	ControlListAddress fragCode = {
		.handle = fragModule->bos[VK_RPI_ASSEMBLY_TYPE_FRAGMENT],
		.offset = 0,
	};

	ControlListAddress vertCode = {
		.handle = vertModule->bos[VK_RPI_ASSEMBLY_TYPE_VERTEX],
		.offset = 0,
	};

	ControlListAddress coordCode = {
		.handle = vertModule->bos[VK_RPI_ASSEMBLY_TYPE_COORDINATE],
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


	//attrib size is simply how many times we read VPM (x4 bytes) in VS and CS
	//attrib records:
	//base address, num bytes, stride are for the kernel side to assemble our vpm
	//VPM offsets: these would be how many vpm reads were before a specific attrib (x4 bytes)
	//we don't really have that info, so we have to play with strides/formats

	uint32_t vertexAttribSize = 0, coordAttribSize = 0;
	for(uint32_t c = 0; c < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++c)
	{
		vertexAttribSize += getFormatBpp(cb->graphicsPipeline->vertexAttributeDescriptions[c].format) >> 3;
		if(cb->graphicsPipeline->vertexAttributeDescriptions[c].location == 0)
		{
			//this should be the vertex coordinates location
			coordAttribSize = getFormatBpp(cb->graphicsPipeline->vertexAttributeDescriptions[c].format) >> 3;
		}
	}

	//number of attribs
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
						 !fragModule->hasThreadSwitch,
						 0, //TODO point size included in shaded vertex data?
						 1, //enable clipping
						 0, //TODO fragment number of used uniforms?
						 fragModule->numVaryings, //fragment number of varyings
						 0, //fragment uniform address?
						 fragCode, //fragment code address
						 0, //TODO vertex number of used uniforms?
						 attribSelectBits, //vertex attribute array select bits
						 vertexAttribSize, //vertex total attribute size
						 0, //vertex uniform address
						 vertCode, //vertex shader code address
						 0, //TODO coordinate number of used uniforms?
						 //TODO how do we know which attribute contains the vertices?
						 //for now the first one will be hardcoded to have the vertices...
						 1 << 0, //coordinate attribute array select bits
						 coordAttribSize, //coordinate total attribute size
						 0, //coordinate uniform address
						 coordCode  //coordinate shader code address
						 );

	uint32_t vertexAttribOffsets[8] = {};
	uint32_t coordAttribOffsets[8] = {};
	for(uint32_t c = 1; c < 8; ++c)
	{
		for(uint32_t d = 0; d < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++d)
		{
			if(cb->graphicsPipeline->vertexAttributeDescriptions[d].location < c)
			{
				vertexAttribOffsets[c] += getFormatBpp(cb->graphicsPipeline->vertexAttributeDescriptions[d].format) >> 3;
			}
		}
	}

	for(uint32_t c = 1; c < 8; ++c)
	{
		coordAttribOffsets[c] = vertexAttribOffsets[1];
	}

	uint32_t maxIndex = 0xffff;
	for(uint32_t c = 0 ; c < cb->graphicsPipeline->vertexAttributeDescriptionCount; ++c)
	{
		if(cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding])
		{
			uint32_t formatByteSize = getFormatBpp(cb->graphicsPipeline->vertexAttributeDescriptions[c].format) >> 3;

			uint32_t stride = cb->graphicsPipeline->vertexBindingDescriptions[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding].stride;

			if(stride > 0)
			{
				uint32_t usedIndices = (cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundMem->size
						- cb->graphicsPipeline->vertexAttributeDescriptions[c].offset
						- vertexOffset * stride
						- cb->vertexBufferOffsets[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]
						- cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundOffset
						- formatByteSize) / stride;

//				fprintf(stderr, "usedIndices %i\n", usedIndices);
//				fprintf(stderr, "boundMemsize %i\n", cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundMem->size);
//				fprintf(stderr, "vertexattrib offset %i\n", cb->graphicsPipeline->vertexAttributeDescriptions[c].offset);
//				fprintf(stderr, "vertex offset %i\n", vertexOffset * stride);
//				fprintf(stderr, "vertex buffer offset %i\n", cb->vertexBufferOffsets[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]);
//				fprintf(stderr, "bound offset %i\n", cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundOffset);
//				fprintf(stderr, "format size %i\n", formatByteSize);
//				fprintf(stderr, "stride %i\n", stride);

				if(usedIndices < maxIndex)
				{
					maxIndex = usedIndices;
				}
			}

			ControlListAddress vertexBuffer = {
				.handle = cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundMem->bo,
				.offset = cb->graphicsPipeline->vertexAttributeDescriptions[c].offset
				+ vertexOffset * stride
				+ cb->vertexBufferOffsets[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]
				+ cb->vertexBuffers[cb->graphicsPipeline->vertexAttributeDescriptions[c].binding]->boundOffset,
			};

			clFit(commandBuffer, &commandBuffer->shaderRecCl, V3D21_ATTRIBUTE_RECORD_length);
			clInsertAttributeRecord(&commandBuffer->shaderRecCl,
									&relocCl,
									&commandBuffer->handlesCl,
									cb->binCl.currMarker->handlesBuf,
									cb->binCl.currMarker->handlesSize,
									vertexBuffer, //reloc address
									formatByteSize,
									stride,
									vertexAttribOffsets[cb->graphicsPipeline->vertexAttributeDescriptions[c].location], //vertex vpm offset
									coordAttribOffsets[cb->graphicsPipeline->vertexAttributeDescriptions[c].location]  //coordinte vpm offset
									);
		}
	}

	//write uniforms
	_pipelineLayout* pl = cb->graphicsPipeline->layout;


	//kernel side expects relocations first!
	for(uint32_t c = 0; c < fragModule->numMappings[VK_RPI_ASSEMBLY_TYPE_FRAGMENT]; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = fragModule->mappings[VK_RPI_ASSEMBLY_TYPE_FRAGMENT][c];

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

	//after relocs we can proceed with the usual uniforms
	for(uint32_t c = 0; c < fragModule->numMappings[VK_RPI_ASSEMBLY_TYPE_FRAGMENT]; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = fragModule->mappings[VK_RPI_ASSEMBLY_TYPE_FRAGMENT][c];

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

				uint32_t cubemapStride = (di->imageView->image->width * di->imageView->image->height * getFormatBpp(di->imageView->interpretedFormat)) >> 3;

				//fprintf(stderr, "cubemap stride %i\n", cubemapStride);

				uint32_t numLevels = 0;
				numLevels = di->imageView->subresourceRange.levelCount < di->imageView->image->miplevels ? di->imageView->subresourceRange.levelCount : di->imageView->image->miplevels;

				uint32_t params[4];
				encodeTextureUniform(params,
									 numLevels - 1,
									 getTextureDataType(di->imageView->interpretedFormat),
									 di->imageView->viewType == VK_IMAGE_VIEW_TYPE_CUBE,
									 cubemapStride >> 12, //cubemap stride in multiples of 4KB
									 (di->imageView->image->levelOffsets[0] + di->imageView->image->boundOffset) >> 12, //Image level 0 offset in multiples of 4KB
									 di->imageView->image->height & 2047,
									 di->imageView->image->width & 2047,
									 getMinFilterType(di->sampler->minFilter, di->sampler->mipmapMode),// di->sampler->maxLod),
									 di->sampler->magFilter == VK_FILTER_NEAREST,
									 getWrapMode(di->sampler->addressModeU),
									 getWrapMode(di->sampler->addressModeV),
									 di->sampler->disableAutoLod
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

				//TMU0_B requires an extra uniform written
				//we need to signal that somehow from API side
				//if mode is cubemap we don't need an extra uniform, it's included!
				if(di->imageView->viewType != VK_IMAGE_VIEW_TYPE_CUBE && di->sampler->disableAutoLod)
				{
					size += 4;
				}

				//emit tex parameters
				clFit(commandBuffer, &commandBuffer->uniformsCl, size);
				clInsertData(&commandBuffer->uniformsCl, size, params);
			}
		}
	}

	//vertex and then coordinate
	for(uint32_t c = 0; c < vertModule->numMappings[VK_RPI_ASSEMBLY_TYPE_VERTEX]; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = vertModule->mappings[VK_RPI_ASSEMBLY_TYPE_VERTEX][c];

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

	//if there are no coordinate mappings, just use the vertex ones
	VkRpiAssemblyTypeEXT coordMappingType = VK_RPI_ASSEMBLY_TYPE_COORDINATE;
	if(vertModule->numMappings[VK_RPI_ASSEMBLY_TYPE_COORDINATE] < 1)
	{
		coordMappingType = VK_RPI_ASSEMBLY_TYPE_VERTEX;
	}

	for(uint32_t c = 0; c < vertModule->numMappings[coordMappingType]; ++c)
	{
		VkRpiAssemblyMappingEXT mapping = vertModule->mappings[coordMappingType][c];

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

	return maxIndex;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdDraw
 */
void rpi_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	assert(commandBuffer);

	if(instanceCount != 1 || firstInstance != 0)
	{
		unsigned instancing;
		UNSUPPORTED(instancing);
	}

	drawCommon(commandBuffer, 0);

	_commandBuffer* cb = commandBuffer;

	//Submit draw call: vertex Array Primitives
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VERTEX_ARRAY_PRIMITIVES_length);
	clInsertVertexArrayPrimitives(&commandBuffer->binCl, firstVertex, vertexCount, getPrimitiveMode(cb->graphicsPipeline->topology));

	cb->numDrawCallsSubmitted++;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndexed(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    indexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstIndex,
	int32_t                                     vertexOffset,
	uint32_t                                    firstInstance)
{
	assert(commandBuffer);

	if(instanceCount != 1 || firstInstance != 0)
	{
		unsigned instancing;
		UNSUPPORTED(instancing);
	}

	uint32_t maxIndex = drawCommon(commandBuffer, vertexOffset);

	_commandBuffer* cb = commandBuffer;

	clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, cb->binCl.currMarker->handlesBuf, cb->binCl.currMarker->handlesSize, cb->indexBuffer->boundMem->bo);

	clInsertGEMRelocations(&commandBuffer->binCl, idx, 0);

	//Submit draw call: vertex Array Primitives
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_VERTEX_ARRAY_PRIMITIVES_length);
	clInsertIndexedPrimitiveList(&commandBuffer->binCl,
								 maxIndex, //max index
								 cb->indexBuffer->boundOffset + cb->indexBufferOffset + firstIndex * 2,
								 indexCount,
								 1, //we only support 16 bit indices
								 getPrimitiveMode(cb->graphicsPipeline->topology));

	cb->numDrawCallsSubmitted++;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndexedIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{
	UNSUPPORTED(vkCmdDrawIndexedIndirect);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{
	UNSUPPORTED(vkCmdDrawIndirect);
}
