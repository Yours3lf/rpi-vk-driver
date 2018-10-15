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
