#include "common.h"

#include "kernel/vc4_packet.h"

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
