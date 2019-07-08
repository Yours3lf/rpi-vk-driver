#include "common.h"

#include "kernel/vc4_packet.h"

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
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateGraphicsPipelines
 */
VkResult vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	assert(device);
	assert(createInfoCount > 0);
	assert(pCreateInfos);
	assert(pPipelines);

	assert(pipelineCache == 0); //TODO not supported right now

	for(int c = 0; c < createInfoCount; ++c)
	{
		_pipeline* pip = ALLOCATE(sizeof(_pipeline), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		for(int d = 0; d < pCreateInfos->stageCount; ++d)
		{
			uint32_t idx = ulog2(pCreateInfos->pStages[d].stage);
			pip->modules[idx] = pCreateInfos->pStages[d].module;

			pip->names[idx] = ALLOCATE(strlen(pCreateInfos->pStages[d].pName)+1, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!pip->names[idx])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			memcpy(pip->names[idx], pCreateInfos->pStages[d].pName, strlen(pCreateInfos->pStages[d].pName)+1);
		}

		pip->vertexAttributeDescriptionCount = pCreateInfos->pVertexInputState->vertexAttributeDescriptionCount;
		pip->vertexAttributeDescriptions = ALLOCATE(sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->vertexAttributeDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexAttributeDescriptions, pCreateInfos->pVertexInputState->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription) * pip->vertexAttributeDescriptionCount);

		pip->vertexBindingDescriptionCount = pCreateInfos->pVertexInputState->vertexBindingDescriptionCount;
		pip->vertexBindingDescriptions = ALLOCATE(sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->vertexBindingDescriptions)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->vertexBindingDescriptions, pCreateInfos->pVertexInputState->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription) * pip->vertexBindingDescriptionCount);

		pip->topology = pCreateInfos->pInputAssemblyState->topology;
		pip->primitiveRestartEnable = pCreateInfos->pInputAssemblyState->primitiveRestartEnable;

		//TODO tessellation ignored

		pip->viewportCount = pCreateInfos->pViewportState->viewportCount;
		pip->viewports = ALLOCATE(sizeof(VkViewport) * pip->viewportCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->viewports)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->viewports, pCreateInfos->pViewportState->pViewports, sizeof(VkViewport) * pip->viewportCount);


		pip->scissorCount = pCreateInfos->pViewportState->scissorCount;
		pip->scissors = ALLOCATE(sizeof(VkRect2D) * pip->viewportCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
		pip->attachmentBlendStates = ALLOCATE(sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pip->attachmentBlendStates)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(pip->attachmentBlendStates, pCreateInfos->pColorBlendState->pAttachments, sizeof(VkPipelineColorBlendAttachmentState) * pip->attachmentCount);

		memcpy(pip->blendConstants, pCreateInfos->pColorBlendState, sizeof(float)*4);


		if(pCreateInfos->pDynamicState)
		{
			pip->dynamicStateCount = pCreateInfos->pDynamicState->dynamicStateCount;
			pip->dynamicStates = ALLOCATE(sizeof(VkDynamicState)*pip->dynamicStateCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	size_t*                                     pDataSize,
	void*                                       pData)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	const VkAllocationCallbacks*                pAllocator)
{

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

}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(
	VkDevice                                    device,
	const VkPipelineCacheCreateInfo*            pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineCache*                            pPipelineCache)
{
	return VK_SUCCESS;
}
