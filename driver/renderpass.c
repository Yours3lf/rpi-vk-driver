#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBeginRenderPass
 */
void rpi_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
	assert(commandBuffer);
	assert(pRenderPassBegin);


	//TODO subpass contents ignored

	_commandBuffer* cb = commandBuffer;
	cb->fbo = pRenderPassBegin->framebuffer;
	cb->renderpass = pRenderPassBegin->renderPass;
	cb->renderArea = pRenderPassBegin->renderArea;

	for(int c = 0; c < pRenderPassBegin->clearValueCount; ++c)
	{
		if(cb->renderpass->attachments[c].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			if(!isDepthStencilFormat(cb->renderpass->attachments[c].format))
			{
				cb->fbo->attachmentViews[c].image->clearColor[0] = cb->fbo->attachmentViews[c].image->clearColor[1] = packVec4IntoABGR8(pRenderPassBegin->pClearValues[c].color.float32);
			}
			else
			{
				//for combined depth/stencil images clearColor 0 is depth and 1 is stencil
				cb->fbo->attachmentViews[c].image->clearColor[0] = (uint32_t)(pRenderPassBegin->pClearValues[c].depthStencil.depth * 0xffffff) & 0xffffff;
			}
		}

		if(isDepthStencilFormat(cb->renderpass->attachments[c].format) && cb->renderpass->attachments[c].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			cb->fbo->attachmentViews[c].image->clearColor[1] = pRenderPassBegin->pClearValues[c].depthStencil.stencil & 0xff;
		}
	}

	cb->currentSubpass = 0;

	_image* i = 0;
	_image* dsI = 0;

	_renderpass* rp = pRenderPassBegin->renderPass;

	for(uint32_t c = 0; c < rp->subpasses[cb->currentSubpass].colorAttachmentCount; ++c)
	{
		i = cb->fbo->attachmentViews[rp->subpasses[cb->currentSubpass].pColorAttachments[c].attachment].image;
		break; //TODO handle multiple attachments
	}

	if(rp->subpasses[cb->currentSubpass].pDepthStencilAttachment)
	{
		dsI = cb->fbo->attachmentViews[rp->subpasses[cb->currentSubpass].pDepthStencilAttachment->attachment].image;
	}

	clFit(commandBuffer, &commandBuffer->binCl, sizeof(CLMarker));
	clInsertNewCLMarker(&commandBuffer->binCl, &cb->handlesCl, &cb->shaderRecCl, cb->shaderRecCount, &cb->uniformsCl, i, dsI);

	//insert reloc for render target
	clFit(commandBuffer, &commandBuffer->handlesCl, 4);
	clGetHandleIndex(&commandBuffer->handlesCl, commandBuffer->binCl.currMarker->handlesBuf, commandBuffer->binCl.currMarker->handlesSize, i->boundMem->bo);

	//insert reloc for depth/stencil image
	if(dsI)
	{
		clFit(commandBuffer, &commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, commandBuffer->binCl.currMarker->handlesBuf, commandBuffer->binCl.currMarker->handlesSize, dsI->boundMem->bo);
	}

	//TODO handle multiple attachments
	for(uint32_t c = 0; c < cb->renderpass->numAttachments; ++c)
	{
		if(cb->renderpass->attachments[c].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			//TODO separate clear for color / depth / stencil?
			cb->binCl.currMarker->flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;
		}
	}

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

	//START_TILE_BINNING resets the statechange counters in the hardware,
	//which are what is used when a primitive is binned to a tile to
	//figure out what new state packets need to be written to that tile's
	//command list.
	clFit(commandBuffer, &commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
	clInsertStartTileBinning(&commandBuffer->binCl);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdEndRenderPass
 */
void rpi_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	//Ending a render pass instance performs any multisample resolve operations on the final subpass

	//Increment the semaphore indicating that binning is done and
	//unblocking the render thread.  Note that this doesn't act
	//until the FLUSH completes.
	//The FLUSH caps all of our bin lists with a
	//VC4_PACKET_RETURN.
	clFit(commandBuffer, &cb->binCl, V3D21_INCREMENT_SEMAPHORE_length);
	clInsertIncrementSemaphore(&cb->binCl);
	clFit(commandBuffer, &cb->binCl, V3D21_FLUSH_length);
	clInsertFlush(&cb->binCl);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateRenderPass
 */
VkResult rpi_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
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

void rpi_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
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
VkResult rpi_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
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

	*pFramebuffer = fb;

	return VK_SUCCESS;
}

void rpi_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
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
VKAPI_ATTR void VKAPI_CALL rpi_vkCmdNextSubpass(
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
VKAPI_ATTR void VKAPI_CALL rpi_vkGetRenderAreaGranularity(
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
