#include "common.h"

#include "declarations.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBeginRenderPass
 */
void RPIFUNC(vkCmdBeginRenderPass)(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
	PROFILESTART(RPIFUNC(vkCmdBeginRenderPass));

	assert(commandBuffer);
	assert(pRenderPassBegin);

//	typedef struct VkRenderPassBeginInfo {
//		VkStructureType        sType;
//		const void*            pNext;
//		VkRenderPass           renderPass;
//		VkFramebuffer          framebuffer;
//		VkRect2D               renderArea;
//		uint32_t               clearValueCount;
//		const VkClearValue*    pClearValues;
//	} VkRenderPassBeginInfo;

	_commandBuffer* cb = commandBuffer;
	_renderpass* rp = pRenderPassBegin->renderPass;
	_framebuffer* fb = pRenderPassBegin->framebuffer;

	_image* writeImage = 0;
	_image* readImage = 0;
	_image* writeDepthStencilImage = 0;
	_image* readDepthStencilImage = 0;
	_image* writeMSAAimage = 0;
	_image* writeMSAAdepthStencilImage = 0;
	uint32_t performResolve = 0;
	uint32_t readMSAAimage = 0;
	uint32_t readMSAAdepthStencilImage = 0;
	uint32_t flags = 0;
	uint32_t writeImageOffset = 0;
	uint32_t readImageOffset = 0;
	uint32_t writeDepthStencilImageOffset = 0;
	uint32_t readDepthStencilImageOffset = 0;
	uint32_t writeMSAAimageOffset = 0;
	uint32_t writeMSAAdepthStencilImageOffset = 0;

	//TODO render area

	//TODO handle multiple subpasses
	//TODO subpass contents ignored
	//TODO input attachments ignored
	//TODO preserve attachments ignored

	//TODO handle lazily allocated memory

	if(rp->subpasses[0].colorAttachmentCount > 0)
	{
		if(rp->subpasses[0].pColorAttachments)
		{
			if(rp->attachments[rp->subpasses[0].pColorAttachments[0].attachment].storeOp == VK_ATTACHMENT_STORE_OP_STORE)
			{
				if(rp->attachments[rp->subpasses[0].pColorAttachments[0].attachment].samples > 1)
				{
					writeMSAAimage = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image;
					writeMSAAimageOffset = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseMipLevel];
					writeMSAAimageOffset += fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->layers;
				}
				else
				{
					writeImage = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image;
					writeImageOffset = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseMipLevel];
					writeImageOffset += fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->layers;
				}
			}

			if(rp->attachments[rp->subpasses[0].pColorAttachments[0].attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
			{
				readImage = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image;
				readImageOffset = fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseMipLevel];
				readImageOffset += fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->layers;

				if(rp->attachments[rp->subpasses[0].pColorAttachments[0].attachment].samples > 1)
				{
					readMSAAimage = 1;
				}
			}
		}

		if(rp->subpasses[0].pResolveAttachments &&
		   rp->attachments[rp->subpasses[0].pResolveAttachments[0].attachment].storeOp == VK_ATTACHMENT_STORE_OP_STORE)
		{
			writeImage = fb->attachmentViews[rp->subpasses[0].pResolveAttachments[0].attachment].image;
			writeImageOffset = fb->attachmentViews[rp->subpasses[0].pResolveAttachments[0].attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pResolveAttachments[0].attachment].subresourceRange.baseMipLevel];
			writeImageOffset += fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->layers;
			performResolve = 1;
		}
	}

	if(rp->subpasses[0].pDepthStencilAttachment)
	{
		if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].storeOp == VK_ATTACHMENT_STORE_OP_STORE ||
		   rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)
		{
			if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].samples > 1)
			{
				writeMSAAdepthStencilImage = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image;
				writeMSAAdepthStencilImageOffset = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseMipLevel];
				writeMSAAdepthStencilImageOffset += fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->layers;
			}
			else
			{
				writeDepthStencilImage = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image;
				writeDepthStencilImageOffset = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseMipLevel];
				writeDepthStencilImageOffset += fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->layers;
			}
		}

		if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
		   rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
		{
			readDepthStencilImage = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image;
			readDepthStencilImageOffset = fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->levelOffsets[fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseMipLevel];
			readDepthStencilImageOffset += fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].subresourceRange.baseArrayLayer * fb->attachmentViews[rp->subpasses[0].pColorAttachments[0].attachment].image->size / fb->attachmentViews[rp->subpasses[0].pDepthStencilAttachment->attachment].image->layers;

			if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].samples > 1)
			{
				readMSAAdepthStencilImage = 1;
			}
		}
	}


	clFit(&commandBuffer->binCl, sizeof(CLMarker));
	clInsertNewCLMarker(&commandBuffer->binCl, &cb->handlesCl, &cb->shaderRecCl, cb->shaderRecCount, &cb->uniformsCl);
	CLMarker* currMarker = getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset);
	currMarker->writeImage = writeImage;
	currMarker->writeImageOffset = writeImageOffset;
	currMarker->readImage = readImage;
	currMarker->readImageOffset = readImageOffset;
	currMarker->writeDepthStencilImage = writeDepthStencilImage;
	currMarker->writeDepthStencilImageOffset = writeDepthStencilImageOffset;
	currMarker->readDepthStencilImage = readDepthStencilImage;
	currMarker->readDepthStencilImageOffset = readDepthStencilImageOffset;
	currMarker->writeMSAAimage = writeMSAAimage;
	currMarker->writeMSAAimageOffset = writeMSAAimageOffset;
	currMarker->writeMSAAdepthStencilImage = writeMSAAdepthStencilImage;
	currMarker->writeMSAAdepthStencilImageOffset = writeMSAAdepthStencilImageOffset;
	currMarker->performResolve = performResolve;
	currMarker->readMSAAimage = readMSAAimage;
	currMarker->readMSAAdepthStencilImage = readMSAAdepthStencilImage;

	if(rp->subpasses[0].colorAttachmentCount > 0)
	{
		if(rp->subpasses[0].pColorAttachments)
		{
			if(rp->attachments[rp->subpasses[0].pColorAttachments[0].attachment].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;

				if(!rp->subpasses[0].pResolveAttachments)
				{
					currMarker->clearColor[0] =
					currMarker->clearColor[1] =
							packVec4IntoABGR8(pRenderPassBegin->pClearValues[rp->subpasses[0].pColorAttachments[0].attachment].color.float32);
				}
				else
				{
					currMarker->clearColor[0] =
					currMarker->clearColor[1] =
							packVec4IntoABGR8(pRenderPassBegin->pClearValues[rp->subpasses[0].pColorAttachments[0].attachment].color.float32);
				}
			}
		}
	}

	if(rp->subpasses[0].pDepthStencilAttachment)
	{
		if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;

			currMarker->clearDepth =
					(uint32_t)(pRenderPassBegin->pClearValues[rp->subpasses[0].pDepthStencilAttachment->attachment].depthStencil.depth * 0xffffff) & 0xffffff;
		}

		if(rp->attachments[rp->subpasses[0].pDepthStencilAttachment->attachment].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;

			currMarker->clearStencil =
					pRenderPassBegin->pClearValues[rp->subpasses[0].pDepthStencilAttachment->attachment].depthStencil.stencil & 0xff;
		}
	}

	currMarker->flags = flags;

	//insert relocs

	if(writeImage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, writeImage->boundMem->bo);
	}

	if(readImage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, readImage->boundMem->bo);
	}

	if(writeDepthStencilImage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, writeDepthStencilImage->boundMem->bo);
	}

	if(readDepthStencilImage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, readDepthStencilImage->boundMem->bo);
	}

	if(writeMSAAimage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, writeMSAAimage->boundMem->bo);
	}

	if(writeMSAAdepthStencilImage)
	{
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesBufOffset + cb->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->handlesSize, writeMSAAdepthStencilImage->boundMem->bo);
	}

	uint32_t bpp = 0;

	((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->width = fb->width;
	((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->height = fb->height;

	if(writeImage)
	{
		bpp = getFormatBpp(writeImage->format);
	}
	else if(writeMSAAimage)
	{
		bpp = getFormatBpp(writeMSAAimage->format);
	}

	uint32_t biggestMip = 0;
	for(uint32_t c = 0; c < fb->numAttachmentViews; ++c)
	{
		biggestMip = max(biggestMip, fb->attachmentViews[c].subresourceRange.baseMipLevel);
	}

	//pad render size if we are rendering to a mip level
	((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->mipLevel = biggestMip;

	uint32_t width = ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->width;

	if(((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->mipLevel > 0)
	{
		width = getPow2Pad(width);
		width = width < 4 ? 4 : width;
	}

	clFit(&commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
	clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
										 0, //double buffer in non ms mode
										 0, //tile allocation block size
										 0, //tile allocation initial block size
										 0, //auto initialize tile state data array
										 bpp == 64, //64 bit color mode
										 writeMSAAimage || writeMSAAdepthStencilImage || performResolve ? 1 : 0, //msaa
										 width, ((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->height,
										 0, //tile state data array address
										 0, //tile allocation memory size
										 0); //tile allocation memory address

	//START_TILE_BINNING resets the statechange counters in the hardware,
	//which are what is used when a primitive is binned to a tile to
	//figure out what new state packets need to be written to that tile's
	//command list.
	clFit(&commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
	clInsertStartTileBinning(&commandBuffer->binCl);

	((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->perfmonID = cb->perfmonID;

	cb->currRenderPass = rp;

	assert(((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->memGuard == 0xDDDDDDDD);

	PROFILEEND(RPIFUNC(vkCmdBeginRenderPass));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdEndRenderPass
 */
void RPIFUNC(vkCmdEndRenderPass)(VkCommandBuffer commandBuffer)
{
	PROFILESTART(RPIFUNC(vkCmdEndRenderPass));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	//Ending a render pass instance performs any multisample resolve operations on the final subpass

	//Increment the semaphore indicating that binning is done and
	//unblocking the render thread.  Note that this doesn't act
	//until the FLUSH completes.
	//The FLUSH caps all of our bin lists with a
	//VC4_PACKET_RETURN.
	clFit(&cb->binCl, V3D21_INCREMENT_SEMAPHORE_length);
	clInsertIncrementSemaphore(&cb->binCl);
	clFit(&cb->binCl, V3D21_FLUSH_length);
	clInsertFlush(&cb->binCl);

	cb->currRenderPass = 0;

	assert(((CLMarker*)getCPAptrFromOffset(cb->binCl.CPA, cb->binCl.currMarkerOffset))->memGuard == 0xDDDDDDDD);

	PROFILEEND(RPIFUNC(vkCmdEndRenderPass));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateRenderPass
 */
VkResult RPIFUNC(vkCreateRenderPass)(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
	PROFILESTART(RPIFUNC(vkCreateRenderPass));

	assert(device);
	assert(pCreateInfo);
	assert(pRenderPass);

	//just copy all data from create info
	//we'll later need to bake the control list based on this

	_renderpass* rp = ALLOCATE(sizeof(_renderpass), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!rp)
	{
		PROFILEEND(RPIFUNC(vkCreateRenderPass));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	rp->numAttachments = pCreateInfo->attachmentCount;
	rp->attachments = ALLOCATE(sizeof(VkAttachmentDescription)*rp->numAttachments, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!rp->attachments)
	{
		PROFILEEND(RPIFUNC(vkCreateRenderPass));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(rp->attachments, pCreateInfo->pAttachments, sizeof(VkAttachmentDescription)*rp->numAttachments);

	rp->numSubpasses = pCreateInfo->subpassCount;
	rp->subpasses = ALLOCATE(sizeof(VkSubpassDescription)*rp->numSubpasses, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!rp->subpasses)
	{
		PROFILEEND(RPIFUNC(vkCreateRenderPass));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(uint32_t c = 0; c < rp->numSubpasses; ++c)
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
				PROFILEEND(RPIFUNC(vkCreateRenderPass));
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
				PROFILEEND(RPIFUNC(vkCreateRenderPass));
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
				PROFILEEND(RPIFUNC(vkCreateRenderPass));
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
				PROFILEEND(RPIFUNC(vkCreateRenderPass));
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
				PROFILEEND(RPIFUNC(vkCreateRenderPass));
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
		PROFILEEND(RPIFUNC(vkCreateRenderPass));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(rp->subpassDependencies, pCreateInfo->pDependencies, sizeof(VkSubpassDependency)*rp->numSubpassDependencies);

	*pRenderPass = rp;

	PROFILEEND(RPIFUNC(vkCreateRenderPass));
	return VK_SUCCESS;
}

void RPIFUNC(vkDestroyRenderPass)(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyRenderPass));

	assert(device);

	_renderpass* rp = renderPass;

	if(rp)
	{
		FREE(rp->subpassDependencies);

		for(uint32_t c = 0; c < rp->numSubpasses; ++c)
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

	PROFILEEND(RPIFUNC(vkDestroyRenderPass));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateFramebuffer
 */
VkResult RPIFUNC(vkCreateFramebuffer)(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	PROFILESTART(RPIFUNC(vkCreateFramebuffer));

	assert(device);
	assert(pCreateInfo);
	assert(pFramebuffer);

	_framebuffer* fb = ALLOCATE(sizeof(_framebuffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!fb)
	{
		PROFILEEND(RPIFUNC(vkCreateFramebuffer));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	fb->renderpass = pCreateInfo->renderPass;

	fb->numAttachmentViews = pCreateInfo->attachmentCount;
	fb->attachmentViews = ALLOCATE(sizeof(_imageView) * fb->numAttachmentViews, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!fb->attachmentViews)
	{
		PROFILEEND(RPIFUNC(vkCreateFramebuffer));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(uint32_t c = 0; c < fb->numAttachmentViews; ++c)
	{
		memcpy(&fb->attachmentViews[c], pCreateInfo->pAttachments[c], sizeof(_imageView));
	}

	fb->width = pCreateInfo->width;
	fb->height = pCreateInfo->height;
	fb->layers = pCreateInfo->layers;

	*pFramebuffer = fb;

	PROFILEEND(RPIFUNC(vkCreateFramebuffer));
	return VK_SUCCESS;
}

void RPIFUNC(vkDestroyFramebuffer)(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyFramebuffer));

	assert(device);

	_framebuffer* fb = framebuffer;
	if(fb)
	{
		FREE(fb->attachmentViews);
		FREE(fb);
	}

	PROFILEEND(RPIFUNC(vkDestroyFramebuffer));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdNextSubpass
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdNextSubpass)(
	VkCommandBuffer                             commandBuffer,
	VkSubpassContents                           contents)
{
	PROFILESTART(RPIFUNC(vkCmdNextSubpass));

	assert(commandBuffer);

	//TODO contents, everything else...

	_commandBuffer* cb = commandBuffer;
	//cb->currentSubpass++; //TODO check max subpass?

	PROFILEEND(RPIFUNC(vkCmdNextSubpass));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetRenderAreaGranularity
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetRenderAreaGranularity)(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	VkExtent2D*                                 pGranularity)
{
	PROFILESTART(RPIFUNC(vkGetRenderAreaGranularity));

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

	PROFILEEND(RPIFUNC(vkGetRenderAreaGranularity));
}
