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

	//just copy all data from create info
	//we'll later need to bake the control list based on this

	_renderpass* rp = ALLOCATE(sizeof(_renderpass), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!rp)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	rp->numAttachments = pCreateInfo->attachmentCount;
	rp->attachments = ALLOCATE(sizeof(VkAttachmentDescription)*rp->numAttachments, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!rp->attachments)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(rp->attachments, pCreateInfo->pAttachments, sizeof(VkAttachmentDescription)*rp->numAttachments);

	rp->numSubpasses = pCreateInfo->subpassCount;
	rp->subpasses = ALLOCATE(sizeof(VkSubpassDescription)*rp->numSubpasses, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
			rp->subpasses[c].pInputAttachments = ALLOCATE(sizeof(VkAttachmentReference)*rp->subpasses[c].inputAttachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
			rp->subpasses[c].pColorAttachments = ALLOCATE(sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
			rp->subpasses[c].pResolveAttachments = ALLOCATE(sizeof(VkAttachmentReference)*rp->subpasses[c].colorAttachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
			rp->subpasses[c].pDepthStencilAttachment = ALLOCATE(sizeof(VkAttachmentReference), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
			rp->subpasses[c].pPreserveAttachments = ALLOCATE(sizeof(uint32_t)*rp->subpasses[c].preserveAttachmentCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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
	rp->subpassDependencies = ALLOCATE(sizeof(VkSubpassDependency)*rp->numSubpassDependencies, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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

	_renderpass* rp = renderPass;

	if(rp)
	{
		FREE(rp->subpassDependencies);

		for(int c = 0; c < rp->numSubpasses; ++c)
		{
			FREE(rp->subpasses[c].pInputAttachments);
			FREE(rp->subpasses[c].pColorAttachments);
			FREE(rp->subpasses[c].pResolveAttachments);
			FREE(rp->subpasses[c].pDepthStencilAttachment);
			FREE(rp->subpasses[c].pPreserveAttachments);
		}

		FREE(rp->subpasses);

		FREE(rp->attachments);

		FREE(rp);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateFramebuffer
 */
VkResult vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pFramebuffer);

	_framebuffer* fb = ALLOCATE(sizeof(_framebuffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!fb)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	fb->renderpass = pCreateInfo->renderPass;

	fb->numAttachmentViews = pCreateInfo->attachmentCount;
	fb->attachmentViews = ALLOCATE(sizeof(_imageView) * fb->numAttachmentViews, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

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

void vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_framebuffer* fb = framebuffer;
	if(fb)
	{
		FREE(fb->attachmentViews);
		FREE(fb);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdNextSubpass
 */
VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(
	VkCommandBuffer                             commandBuffer,
	VkSubpassContents                           contents)
{
	assert(commandBuffer);

	//TODO contents, everything else...

	_commandBuffer* cb = commandBuffer;
	cb->currentSubpass++; //TODO check max subpass?
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetRenderAreaGranularity
 */
VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	VkExtent2D*                                 pGranularity)
{
	assert(device);
	assert(renderPass);
	assert(pGranularity);

	_renderpass* rp = renderPass;

	//TODO what if we have multiple attachments?

	uint32_t tileSizeW = 64;
	uint32_t tileSizeH = 64;

	if(rp->attachments[0].samples > 1)
	{
		tileSizeW >>= 1;
		tileSizeH >>= 1;
	}

	if(getFormatBpp(rp->attachments[0].format) == 64)
	{
		tileSizeH >>= 1;
	}

	pGranularity->width = tileSizeW;
	pGranularity->height = tileSizeH;
}
