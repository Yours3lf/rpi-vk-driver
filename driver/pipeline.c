#include "common.h"

#include "kernel/vc4_packet.h"
#include "../QPUassembler/qpu_assembler.h"

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

void patchShaderDepthStencilBlending(uint64_t** instructions, uint32_t* size, const VkPipelineDepthStencilStateCreateInfo* dsi, const VkAllocationCallbacks* pAllocator)
{
	assert(instructions);
	assert(size);
	assert(dsi);

	uint32_t numExtraInstructions = 0;
	numExtraInstructions += dsi->depthWriteEnable || dsi->stencilTestEnable;

	uint32_t values[3];
	uint32_t numValues;
	encodeStencilValue(values, &numValues, dsi->front, dsi->back, dsi->stencilTestEnable);

	numExtraInstructions += numValues * 2;

	uint32_t newSize = *size + numExtraInstructions * sizeof(uint64_t);
	uint64_t* tmp = ALLOCATE(newSize, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	memset(tmp, 0, newSize);
	memcpy(tmp + numExtraInstructions, *instructions, *size);

	///"sig_load_imm ; r0 = load32.always(0xF497EEFF‬) ; nop = load32() ;" //stencil setup state
	///"sig_none ; tlb_stencil_setup = or.always(r0, r0) ; nop = nop(r0, r0) ;"
	for(uint32_t c = 0; c < numValues; ++c)
	{
		tmp[c] = encode_load_imm(0, 0, 1, 0, 0, 0, 32 + c, 39, values[c]); //r0 = load32.always(values[c])
		tmp[numValues + c] = encode_alu(1, 0, 0, 0, 1, 0, 0, 0, 43, 39, 0, 21, 0, 0, c, c, 0, 0); //tlb_stencil_setup = or.always(r0, r0)
	}

	///"sig_none ; tlb_z = or.always(b, b, nop, rb15) ; nop = nop(r0, r0) ;"
	if(dsi->depthWriteEnable || dsi->stencilTestEnable)
	{
		tmp[numValues*2] = encode_alu(1, 0, 0, 0, 1, 0, 0, 0, 44, 39, 0, 21, 0, 15, 7, 7, 0, 0);
	}

	//replace instructions pointer
	FREE(*instructions);
	*instructions = tmp;
	*size = newSize;
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

	if(pipelineCache)
	{
		UNSUPPORTED(pipelineCache);
	}

	//TODO pipeline caches
	//TODO flags

	for(int c = 0; c < createInfoCount; ++c)
	{
		_pipeline* pip = ALLOCATE(sizeof(_pipeline), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memset(pip->names, 0, sizeof(char*)*6);
		memset(pip->modules, 0, sizeof(_shaderModule*)*6);

		for(int d = 0; d < pCreateInfos[c].stageCount; ++d)
		{
			uint32_t idx = ulog2(pCreateInfos[c].pStages[d].stage);
			pip->modules[idx] = pCreateInfos[c].pStages[d].module;

			_shaderModule* s = pip->modules[idx];

			pip->names[idx] = ALLOCATE(strlen(pCreateInfos[c].pStages[d].pName)+1, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!pip->names[idx])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(pip->names[idx], pCreateInfos[c].pStages[d].pName, strlen(pCreateInfos[c].pStages[d].pName)+1);

			//patch fragment shader
			if(pCreateInfos[c].pStages[d].stage & VK_SHADER_STAGE_FRAGMENT_BIT)
			{
				patchShaderDepthStencilBlending(&s->instructions[RPI_ASSEMBLY_TYPE_FRAGMENT], &s->sizes[RPI_ASSEMBLY_TYPE_FRAGMENT], pCreateInfos[c].pDepthStencilState, pAllocator);

				//TODO if debug...
				for(uint64_t e = 0; e < s->sizes[RPI_ASSEMBLY_TYPE_FRAGMENT] / 8; ++e)
				{
					printf("%#llx ", s->instructions[RPI_ASSEMBLY_TYPE_FRAGMENT][e]);
					disassemble_qpu_asm(s->instructions[RPI_ASSEMBLY_TYPE_FRAGMENT][e]);
				}

				printf("\n");

				s->bos[RPI_ASSEMBLY_TYPE_FRAGMENT] = vc4_bo_alloc_shader(controlFd, s->instructions[RPI_ASSEMBLY_TYPE_FRAGMENT], &s->sizes[RPI_ASSEMBLY_TYPE_FRAGMENT]);
			}

			if(pCreateInfos[c].pStages[d].stage & VK_SHADER_STAGE_VERTEX_BIT)
			{
				//TODO if debug...
				for(uint64_t e = 0; e < s->sizes[RPI_ASSEMBLY_TYPE_VERTEX] / 8; ++e)
				{
					printf("%#llx ", s->instructions[RPI_ASSEMBLY_TYPE_VERTEX][e]);
					disassemble_qpu_asm(s->instructions[RPI_ASSEMBLY_TYPE_VERTEX][e]);
				}

				printf("\n");

				for(uint64_t e = 0; e < s->sizes[RPI_ASSEMBLY_TYPE_COORDINATE] / 8; ++e)
				{
					printf("%#llx ", s->instructions[RPI_ASSEMBLY_TYPE_COORDINATE][e]);
					disassemble_qpu_asm(s->instructions[RPI_ASSEMBLY_TYPE_COORDINATE][e]);
				}

				printf("\n");

				s->bos[RPI_ASSEMBLY_TYPE_COORDINATE] = vc4_bo_alloc_shader(controlFd, s->instructions[RPI_ASSEMBLY_TYPE_COORDINATE], &s->sizes[RPI_ASSEMBLY_TYPE_COORDINATE]);
				s->bos[RPI_ASSEMBLY_TYPE_VERTEX] = vc4_bo_alloc_shader(controlFd, s->instructions[RPI_ASSEMBLY_TYPE_VERTEX], &s->sizes[RPI_ASSEMBLY_TYPE_VERTEX]);
			}
		}

		pip->vertexAttributeDescriptionCount = pCreateInfos[c].pVertexInputState->vertexAttributeDescriptionCount;
		pip->vertexAttributeDescriptions = ALLOCATE(sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->vertexAttributeDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexAttributeDescriptions, pCreateInfos[c].pVertexInputState->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount);

		pip->vertexBindingDescriptionCount = pCreateInfos[c].pVertexInputState->vertexBindingDescriptionCount;
		pip->vertexBindingDescriptions = ALLOCATE(sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->vertexBindingDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexBindingDescriptions, pCreateInfos[c].pVertexInputState->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount);

		pip->topology = pCreateInfos[c].pInputAssemblyState->topology;
		pip->primitiveRestartEnable = pCreateInfos[c].pInputAssemblyState->primitiveRestartEnable;

		//tessellation ignored

		pip->viewportCount = pCreateInfos[c].pViewportState->viewportCount;
		pip->viewports = ALLOCATE(sizeof(VkViewport) * pip->viewportCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->viewports)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->viewports, pCreateInfos[c].pViewportState->pViewports, sizeof(VkViewport) * pip->viewportCount);


		pip->scissorCount = pCreateInfos[c].pViewportState->scissorCount;
		pip->scissors = ALLOCATE(sizeof(VkRect2D) * pip->viewportCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->scissors)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->scissors, pCreateInfos[c].pViewportState->pScissors, sizeof(VkRect2D) * pip->scissorCount);

		pip->depthClampEnable = pCreateInfos[c].pRasterizationState->depthClampEnable;
		pip->rasterizerDiscardEnable = pCreateInfos[c].pRasterizationState->rasterizerDiscardEnable;
		pip->polygonMode = pCreateInfos[c].pRasterizationState->polygonMode;
		pip->cullMode = pCreateInfos[c].pRasterizationState->cullMode;
		pip->frontFace = pCreateInfos[c].pRasterizationState->frontFace;
		pip->depthBiasEnable = pCreateInfos[c].pRasterizationState->depthBiasEnable;
		pip->depthBiasConstantFactor = pCreateInfos[c].pRasterizationState->depthBiasConstantFactor;
		pip->depthBiasClamp = pCreateInfos[c].pRasterizationState->depthBiasClamp;
		pip->depthBiasSlopeFactor = pCreateInfos[c].pRasterizationState->depthBiasSlopeFactor;
		pip->lineWidth = pCreateInfos[c].pRasterizationState->lineWidth;

		pip->rasterizationSamples = pCreateInfos[c].pMultisampleState->rasterizationSamples;
		pip->sampleShadingEnable = pCreateInfos[c].pMultisampleState->sampleShadingEnable;
		pip->minSampleShading = pCreateInfos[c].pMultisampleState->minSampleShading;
		if(pCreateInfos[c].pMultisampleState->pSampleMask)
		{
			pip->sampleMask = *pCreateInfos[c].pMultisampleState->pSampleMask;
		}
		else
		{
			pip->sampleMask = 0;
		}
		pip->alphaToCoverageEnable = pCreateInfos[c].pMultisampleState->alphaToCoverageEnable;
		pip->alphaToOneEnable = pCreateInfos[c].pMultisampleState->alphaToOneEnable;

		pip->depthTestEnable = pCreateInfos[c].pDepthStencilState->depthTestEnable;
		pip->depthWriteEnable = pCreateInfos[c].pDepthStencilState->depthWriteEnable;
		pip->depthCompareOp = pCreateInfos[c].pDepthStencilState->depthCompareOp;
		pip->depthBoundsTestEnable = pCreateInfos[c].pDepthStencilState->depthBoundsTestEnable;
		pip->stencilTestEnable = pCreateInfos[c].pDepthStencilState->stencilTestEnable;
		pip->front = pCreateInfos[c].pDepthStencilState->front;
		pip->back = pCreateInfos[c].pDepthStencilState->back;
		pip->minDepthBounds = pCreateInfos[c].pDepthStencilState->minDepthBounds;
		pip->maxDepthBounds = pCreateInfos[c].pDepthStencilState->maxDepthBounds;

		pip->logicOpEnable = pCreateInfos[c].pColorBlendState->logicOpEnable;
		pip->logicOp = pCreateInfos[c].pColorBlendState->logicOp;
		pip->attachmentCount = pCreateInfos[c].pColorBlendState->attachmentCount;
		pip->attachmentBlendStates = ALLOCATE(sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->attachmentBlendStates)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->attachmentBlendStates, pCreateInfos[c].pColorBlendState->pAttachments, sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount);

		memcpy(pip->blendConstants, pCreateInfos[c].pColorBlendState, sizeof(float)*4);


		if(pCreateInfos[c].pDynamicState)
		{
			pip->dynamicStateCount = pCreateInfos[c].pDynamicState->dynamicStateCount;
			pip->dynamicStates = ALLOCATE(sizeof(VkDynamicState)*pip->dynamicStateCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!pip->dynamicStates)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(pip->dynamicStates, pCreateInfos[c].pDynamicState->pDynamicStates, sizeof(VkDynamicState)*pip->dynamicStateCount);
		}
		else
		{
			pip->dynamicStateCount = 0;
			pip->dynamicStates = 0;
		}

		pip->layout = pCreateInfos[c].layout;
		pip->renderPass = pCreateInfos[c].renderPass;
		pip->subpass = pCreateInfos[c].subpass;

		//TODO derivative pipelines ignored

		pPipelines[c] = pip;
	}

	return VK_SUCCESS;
}

void vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_pipeline* pip = pipeline;

	if(pip)
	{
		FREE(pip->dynamicStates);
		FREE(pip->attachmentBlendStates);
		FREE(pip->scissors);
		FREE(pip->viewports);
		FREE(pip->vertexBindingDescriptions);
		FREE(pip->vertexAttributeDescriptions);

		for(int c = 0; c < 6; ++c)
		{
			FREE(pip->names[c]);
		}
		FREE(pip);
	}
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(
	VkDevice                                    device,
	VkPipelineCache                             dstCache,
	uint32_t                                    srcCacheCount,
	const VkPipelineCache*                      pSrcCaches)
{
	UNSUPPORTED(vkMergePipelineCaches);
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	size_t*                                     pDataSize,
	void*                                       pData)
{
	UNSUPPORTED(vkGetPipelineCacheData);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	const VkAllocationCallbacks*                pAllocator)
{
	UNSUPPORTED(vkDestroyPipelineCache);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(
	VkDevice                                    device,
	const VkPipelineLayoutCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineLayout*                           pPipelineLayout)
{
	assert(device);
	assert(pCreateInfo);
	assert(pPipelineLayout);

	_pipelineLayout* pl = ALLOCATE(sizeof(_pipelineLayout), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!pl)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	pl->setLayoutCount = pCreateInfo->setLayoutCount;
	pl->pushConstantRangeCount = pCreateInfo->pushConstantRangeCount;

	if(pCreateInfo->setLayoutCount > 0 && pCreateInfo->pSetLayouts)
	{
		pl->setLayouts = ALLOCATE(sizeof(VkDescriptorSetLayout)*pCreateInfo->setLayoutCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pl->setLayouts)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pl->setLayouts, pCreateInfo->pSetLayouts, sizeof(VkDescriptorSetLayout)*pCreateInfo->setLayoutCount);
	}

	if(pCreateInfo->pushConstantRangeCount > 0 && pCreateInfo->pPushConstantRanges)
	{
		pl->pushConstantRanges = ALLOCATE(sizeof(VkPushConstantRange)*pCreateInfo->pushConstantRangeCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pl->pushConstantRanges)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pl->pushConstantRanges, pCreateInfo->pPushConstantRanges, sizeof(VkPushConstantRange)*pCreateInfo->pushConstantRangeCount);
	}

	pl->descriptorSetBindingMap = createMap(ALLOCATE(sizeof(_descriptorSet*)*pCreateInfo->setLayoutCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT), pCreateInfo->setLayoutCount);

	*pPipelineLayout = pl;

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(
	VkDevice                                    device,
	VkPipelineLayout                            pipelineLayout,
	const VkAllocationCallbacks*                pAllocator)
{
	//TODO
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(
	VkDevice                                    device,
	const VkPipelineCacheCreateInfo*            pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineCache*                            pPipelineCache)
{
	UNSUPPORTED(vkCreatePipelineCache);
	return VK_SUCCESS;
}
