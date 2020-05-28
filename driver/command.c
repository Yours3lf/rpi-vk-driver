#include "common.h"

#include "kernel/vc4_packet.h"
#include "../brcm/cle/v3d_decoder.h"
#include "../brcm/clif/clif_dump.h"

#include "declarations.h"

#include <stdatomic.h>

static uint64_t lastFinishedSeqno = 0;
static atomic_int lastSeqnoGuard = 0;

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffers-pools
 * Command pools are opaque objects that command buffer memory is allocated from, and which allow the implementation to amortize the
 * cost of resource creation across multiple command buffers. Command pools are externally synchronized, meaning that a command pool must
 * not be used concurrently in multiple threads. That includes use via recording commands on any command buffers allocated from the pool,
 * as well as operations that allocate, free, and reset command buffers or the pool itself.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateCommandPool)(
		VkDevice                                    device,
		const VkCommandPoolCreateInfo*              pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkCommandPool*                              pCommandPool)
{
	PROFILESTART(RPIFUNC(vkCreateCommandPool));

	assert(device);
	assert(pCreateInfo);

	//TODO VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	//specifies that command buffers allocated from the pool will be short-lived, meaning that they will be reset or freed in a relatively short timeframe.
	//This flag may be used by the implementation to control memory allocation behavior within the pool.
	//--> definitely use pool allocator

	//TODO pool family ignored for now

	_commandPool* cp = ALLOCATE(sizeof(_commandPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!cp)
	{
		PROFILEEND(RPIFUNC(vkCreateCommandPool));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	cp->queueFamilyIndex = pCreateInfo->queueFamilyIndex;

	cp->resetAble = pCreateInfo->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;


	//initial number of command buffers to hold
	int numCommandBufs = 128;
	//TODO uniforms might need to realloc, which should be handled properly
	int consecutiveBlockSize = ARM_PAGE_SIZE * 20;
	int consecutiveBlockNumber = 256;
	//int numCommandBufs = 30;
	//int consecutiveBlockSize = getCPABlockSize(256);
	//int consecutiveBlockNumber = 30;
	int consecutivePoolSize = consecutiveBlockNumber * consecutiveBlockSize;

	static int counter = 0;

	//if(pCreateInfo->flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	{
		//use pool allocator
		void* pamem = ALLOCATE(numCommandBufs * sizeof(_commandBuffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pamem)
		{
			PROFILEEND(RPIFUNC(vkCreateCommandPool));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->pa = createPoolAllocator(pamem, sizeof(_commandBuffer), numCommandBufs * sizeof(_commandBuffer));

		void* cpamem = ALLOCATE(consecutivePoolSize, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!cpamem)
		{
			PROFILEEND(RPIFUNC(vkCreateCommandPool));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->cpa = createConsecutivePoolAllocator(cpamem, consecutiveBlockSize, consecutivePoolSize);
	}

	*pCommandPool = (VkCommandPool)cp;

	PROFILEEND(RPIFUNC(vkCreateCommandPool));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffer-allocation
 * vkAllocateCommandBuffers can be used to create multiple command buffers. If the creation of any of those command buffers fails,
 * the implementation must destroy all successfully created command buffer objects from this command, set all entries of the pCommandBuffers array to NULL and return the error.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAllocateCommandBuffers)(
		VkDevice                                    device,
		const VkCommandBufferAllocateInfo*          pAllocateInfo,
		VkCommandBuffer*                            pCommandBuffers)
{
	PROFILESTART(RPIFUNC(vkAllocateCommandBuffers));

	assert(device);
	assert(pAllocateInfo);
	assert(pCommandBuffers);

	VkResult res = VK_SUCCESS;

	_commandPool* cp = (_commandPool*)pAllocateInfo->commandPool;

	//TODO secondary command buffers

	//if(cp->usePoolAllocator)
	{
		for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
		{
			pCommandBuffers[c] = poolAllocate(&cp->pa);

			if(!pCommandBuffers[c])
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			set_loader_magic_value(&pCommandBuffers[c]->loaderData);

			pCommandBuffers[c]->dev = device;

			pCommandBuffers[c]->shaderRecCount = 0;
			pCommandBuffers[c]->usageFlags = 0;
			pCommandBuffers[c]->state = CMDBUF_STATE_INITIAL;
			pCommandBuffers[c]->cp = cp;
			clInit(&pCommandBuffers[c]->binCl, &cp->cpa, consecutivePoolAllocate(&cp->cpa, 1), cp->cpa.blockSize);
			clInit(&pCommandBuffers[c]->handlesCl, &cp->cpa, consecutivePoolAllocate(&cp->cpa, 1), cp->cpa.blockSize);
			clInit(&pCommandBuffers[c]->shaderRecCl, &cp->cpa, consecutivePoolAllocate(&cp->cpa, 1), cp->cpa.blockSize);
			clInit(&pCommandBuffers[c]->uniformsCl, &cp->cpa, consecutivePoolAllocate(&cp->cpa, 1), cp->cpa.blockSize);

			pCommandBuffers[c]->graphicsPipeline = 0;
			pCommandBuffers[c]->computePipeline = 0;
			pCommandBuffers[c]->numDrawCallsSubmitted = 0;
			pCommandBuffers[c]->indexBuffer = 0;
			pCommandBuffers[c]->indexBufferOffset = 0;
			pCommandBuffers[c]->vertexBufferDirty = 1;
			pCommandBuffers[c]->indexBufferDirty = 1;
			pCommandBuffers[c]->viewportDirty = 1;
			pCommandBuffers[c]->lineWidthDirty = 1;
			pCommandBuffers[c]->depthBiasDirty = 1;
			pCommandBuffers[c]->graphicsPipelineDirty = 1;
			pCommandBuffers[c]->computePipelineDirty = 1;
			pCommandBuffers[c]->subpassDirty = 1;
			pCommandBuffers[c]->blendConstantsDirty = 1;
			pCommandBuffers[c]->scissorDirty = 1;
			pCommandBuffers[c]->depthBoundsDirty = 1;
			pCommandBuffers[c]->stencilCompareMaskDirty = 1;
			pCommandBuffers[c]->stencilWriteMaskDirty = 1;
			pCommandBuffers[c]->stencilReferenceDirty = 1;
			pCommandBuffers[c]->descriptorSetDirty = 1;
			pCommandBuffers[c]->pushConstantDirty = 1;
			pCommandBuffers[c]->currRenderPass = 0;

			pCommandBuffers[c]->perfmonID = 0;

			if(pCommandBuffers[c]->binCl.offset == -1)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(pCommandBuffers[c]->handlesCl.offset == -1)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(pCommandBuffers[c]->shaderRecCl.offset == -1)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(pCommandBuffers[c]->uniformsCl.offset == -1)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}
		}
	}

	if(res != VK_SUCCESS)
	{
		//if(cp->usePoolAllocator)
		{
			for(int c = 0; c < pAllocateInfo->commandBufferCount; ++c)
			{
				consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->binCl.offset), pCommandBuffers[c]->binCl.numBlocks);
				consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->handlesCl.offset), pCommandBuffers[c]->handlesCl.numBlocks);
				consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->shaderRecCl.offset), pCommandBuffers[c]->shaderRecCl.numBlocks);
				consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->uniformsCl.offset), pCommandBuffers[c]->uniformsCl.numBlocks);
				poolFree(&cp->pa, pCommandBuffers[c]);
				pCommandBuffers[c] = 0;
			}
		}
	}

	PROFILEEND(RPIFUNC(vkAllocateCommandBuffers));
	return res;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBeginCommandBuffer
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBeginCommandBuffer)(
		VkCommandBuffer                             commandBuffer,
		const VkCommandBufferBeginInfo*             pBeginInfo)
{
	PROFILESTART(RPIFUNC(vkBeginCommandBuffer));

	assert(commandBuffer);
	assert(pBeginInfo);

	//TODO secondary command buffers

	//TODO VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
	//specifies that a secondary command buffer is considered to be entirely inside a render pass. If this is a primary command buffer, then this bit is ignored

	//TODO VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
	//specifies that a command buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary command buffers

	//When a command buffer begins recording, all state in that command buffer is undefined

	commandBuffer->usageFlags = pBeginInfo->flags;
	commandBuffer->state = CMDBUF_STATE_RECORDING;

	if((pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) &&
	   commandBuffer->cp->resetAble)
	{
		RPIFUNC(vkResetCommandBuffer)(commandBuffer, 0);
	}

	PROFILEEND(RPIFUNC(vkBeginCommandBuffer));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEndCommandBuffer
 * If there was an error during recording, the application will be notified by an unsuccessful return code returned by vkEndCommandBuffer.
 * If the application wishes to further use the command buffer, the command buffer must be reset. The command buffer must have been in the recording state,
 * and is moved to the executable state.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEndCommandBuffer)(
		VkCommandBuffer                             commandBuffer)
{
	PROFILESTART(RPIFUNC(vkEndCommandBuffer));

	assert(commandBuffer);

	commandBuffer->state = CMDBUF_STATE_EXECUTABLE;

	PROFILEEND(RPIFUNC(vkEndCommandBuffer));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkQueueSubmit
 * vkQueueSubmit is a queue submission command, with each batch defined by an element of pSubmits as an instance of the VkSubmitInfo structure.
 * Batches begin execution in the order they appear in pSubmits, but may complete out of order.
 * Fence and semaphore operations submitted with vkQueueSubmit have additional ordering constraints compared to other submission commands,
 * with dependencies involving previous and subsequent queue operations. Information about these additional constraints can be found in the semaphore and
 * fence sections of the synchronization chapter.
 * Details on the interaction of pWaitDstStageMask with synchronization are described in the semaphore wait operation section of the synchronization chapter.
 * The order that batches appear in pSubmits is used to determine submission order, and thus all the implicit ordering guarantees that respect it.
 * Other than these implicit ordering guarantees and any explicit synchronization primitives, these batches may overlap or otherwise execute out of order.
 * If any command buffer submitted to this queue is in the executable state, it is moved to the pending state. Once execution of all submissions of a command buffer complete,
 * it moves from the pending state, back to the executable state. If a command buffer was recorded with the VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT flag,
 * it instead moves back to the invalid state.
 * If vkQueueSubmit fails, it may return VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY.
 * If it does, the implementation must ensure that the state and contents of any resources or synchronization primitives referenced by the submitted command buffers and any semaphores
 * referenced by pSubmits is unaffected by the call or its failure. If vkQueueSubmit fails in such a way that the implementation is unable to make that guarantee,
 * the implementation must return VK_ERROR_DEVICE_LOST. See Lost Device.
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueueSubmit)(
		VkQueue                                     queue,
		uint32_t                                    submitCount,
		const VkSubmitInfo*                         pSubmits,
		VkFence                                     fence)
{
	PROFILESTART(RPIFUNC(vkQueueSubmit));

	assert(queue);

	//TODO this is incorrect
	//see sync.c
	//TODO: deal with pSubmits->pWaitDstStageMask
	for(int c = 0; c < pSubmits->waitSemaphoreCount; ++c)
	{
		sem_wait((sem_t*)pSubmits->pWaitSemaphores[c]);
	}

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		if(pSubmits->pCommandBuffers[c]->state == CMDBUF_STATE_EXECUTABLE)
		{
			pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_PENDING;
		}
	}

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		VkCommandBuffer cmdbuf = pSubmits->pCommandBuffers[c];

		if(cmdbuf->binCl.currMarkerOffset == -1)
		{
			//no markers recorded yet, skip
			continue;
		}

		//first entry is assumed to be a marker
		CLMarker* marker = getCPAptrFromOffset(cmdbuf->binCl.CPA, cmdbuf->binCl.offset);

		//a command buffer may contain multiple render passes
		//and commands outside render passes such as clear commands
		//each of these corresponds to a control list submit

		//submit each separate control list
		while(marker)
		{
			assert(marker->memGuard == 0xDDDDDDDD);

			struct drm_vc4_submit_cl submitCl =
			{
				.color_read.hindex = ~0,
				.zs_read.hindex = ~0,
				.color_write.hindex = ~0,
				.msaa_color_write.hindex = ~0,
				.zs_write.hindex = ~0,
				.msaa_zs_write.hindex = ~0,
			};

			_image* writeImage = marker->writeImage;
			_image* readImage = marker->readImage;
			_image* writeDepthStencilImage = marker->writeDepthStencilImage;
			_image* readDepthStencilImage = marker->readDepthStencilImage;
			_image* writeMSAAimage = marker->writeMSAAimage;
			_image* writeMSAAdepthStencilImage = marker->writeMSAAdepthStencilImage;
			uint32_t performResolve = marker->performResolve;
			uint32_t readMSAAimage = marker->readMSAAimage;
			uint32_t readMSAAdepthStencilImage = marker->readMSAAdepthStencilImage;

			//This should not result in an insertion!
			clFit(cmdbuf, &cmdbuf->handlesCl, 4 * 6); //just to be safe
			uint32_t writeImageIdx = writeImage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBufOffset + cmdbuf->handlesCl.offset, marker->handlesSize, writeImage->boundMem->bo) : 0;
			uint32_t readImageIdx = readImage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBufOffset + cmdbuf->handlesCl.offset, marker->handlesSize, readImage->boundMem->bo) : 0;
			uint32_t writeDepthStencilImageIdx = writeDepthStencilImage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBufOffset + cmdbuf->handlesCl.offset, marker->handlesSize, writeDepthStencilImage->boundMem->bo) : 0;
			uint32_t readDepthStencilImageIdx = readDepthStencilImage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBufOffset + cmdbuf->handlesCl.offset, marker->handlesSize, readDepthStencilImage->boundMem->bo) : 0;
			uint32_t writeMSAAimageIdx = writeMSAAimage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesSize, marker->handlesBufOffset + cmdbuf->handlesCl.offset, writeMSAAimage->boundMem->bo) : 0;
			uint32_t writeMSAAdepthStencilImageIdx = writeMSAAdepthStencilImage ? clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBufOffset + cmdbuf->handlesCl.offset, marker->handlesSize, writeMSAAdepthStencilImage->boundMem->bo) : 0;

			submitCl.clear_color[0] = 0;
			submitCl.clear_color[1] = 0;
			submitCl.clear_z = 0;
			submitCl.clear_s = 0;

			//fill out submit cl fields
			if(writeImage)
			{
				uint32_t tiling = writeImage->tiling;

				if(marker->mipLevel > 0)
				{
					tiling = writeImage->levelTiling[marker->mipLevel];
				}

				submitCl.color_write.hindex = writeImageIdx;
				submitCl.color_write.offset = marker->writeImageOffset + writeImage->boundOffset;
				submitCl.color_write.flags = 0;
				submitCl.color_write.bits =
						VC4_SET_FIELD(getRenderTargetFormatVC4(writeImage->format), VC4_RENDER_CONFIG_FORMAT) |
						VC4_SET_FIELD(tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);

				if(performResolve)
				{
					submitCl.color_write.bits |= VC4_RENDER_CONFIG_MS_MODE_4X | VC4_RENDER_CONFIG_DECIMATE_MODE_4X;
				}
			}

			if(writeMSAAimage)
			{
				submitCl.msaa_color_write.hindex = writeMSAAimageIdx;
				submitCl.msaa_color_write.offset = marker->writeMSAAimageOffset + writeMSAAimage->boundOffset;
				submitCl.msaa_color_write.flags = 0;
				submitCl.msaa_color_write.bits = VC4_RENDER_CONFIG_MS_MODE_4X;
			}

			if(readImage)
			{
				uint32_t tiling = readImage->tiling;

				if(marker->mipLevel > 0)
				{
					tiling = readImage->levelTiling[marker->mipLevel];
				}

				submitCl.color_read.hindex = readImageIdx;
				submitCl.color_read.offset = marker->readImageOffset + readImage->boundOffset;
				submitCl.color_read.flags = readMSAAimage ? VC4_SUBMIT_RCL_SURFACE_READ_IS_FULL_RES : 0;
				submitCl.color_read.bits = VC4_SET_FIELD(getRenderTargetFormatVC4(readImage->format), VC4_RENDER_CONFIG_FORMAT) |
						VC4_SET_FIELD(tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);
			}

			if(writeDepthStencilImage)
			{
				uint32_t tiling = writeDepthStencilImage->tiling;

				if(marker->mipLevel > 0)
				{
					tiling = writeDepthStencilImage->levelTiling[marker->mipLevel];
				}

				submitCl.zs_write.hindex = writeDepthStencilImageIdx;
				submitCl.zs_write.offset = marker->writeDepthStencilImageOffset + writeDepthStencilImage->boundOffset;
				submitCl.zs_write.flags = 0;
				submitCl.zs_write.bits = VC4_SET_FIELD(VC4_LOADSTORE_TILE_BUFFER_ZS, VC4_LOADSTORE_TILE_BUFFER_BUFFER) |
										 VC4_SET_FIELD(tiling, VC4_LOADSTORE_TILE_BUFFER_TILING);
			}

			if(writeMSAAdepthStencilImage)
			{
				submitCl.msaa_zs_write.hindex = writeMSAAdepthStencilImageIdx;
				submitCl.msaa_zs_write.offset = marker->writeMSAAdepthStencilImageOffset + writeMSAAdepthStencilImage->boundOffset;
				submitCl.msaa_zs_write.flags = 0;
				submitCl.msaa_zs_write.bits = VC4_RENDER_CONFIG_MS_MODE_4X;
			}

			if(readDepthStencilImage)
			{
				uint32_t tiling = readDepthStencilImage->tiling;

				if(marker->mipLevel > 0)
				{
					tiling = readDepthStencilImage->levelTiling[marker->mipLevel];
				}

				submitCl.zs_read.hindex = readDepthStencilImageIdx;
				submitCl.zs_read.offset = marker->readDepthStencilImageOffset + readDepthStencilImage->boundOffset;
				submitCl.zs_read.flags = readMSAAdepthStencilImage ? VC4_SUBMIT_RCL_SURFACE_READ_IS_FULL_RES : 0; //TODO is this valid?
				submitCl.zs_read.bits = VC4_SET_FIELD(getRenderTargetFormatVC4(readDepthStencilImage->format), VC4_RENDER_CONFIG_FORMAT) |
						VC4_SET_FIELD(tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);
			}

			submitCl.clear_color[0] = marker->clearColor[0];
			submitCl.clear_color[1] = marker->clearColor[1];

			submitCl.clear_z = marker->clearDepth; //0...1 -> 0...0xffffff
			submitCl.clear_s = marker->clearStencil; //0...0xff


//			fprintf(stderr, "submitCl.clear_color[0]: %u\n", submitCl.clear_color[0]);
//			fprintf(stderr, "submitCl.clear_color[1]: %u\n", submitCl.clear_color[1]);
//			fprintf(stderr, "submitCl.clear_z: %u\n", submitCl.clear_z);
//			fprintf(stderr, "submitCl.clear_s: %u\n", submitCl.clear_s);

			submitCl.min_x_tile = 0;
			submitCl.min_y_tile = 0;

			uint32_t tileSizeW = 64;
			uint32_t tileSizeH = 64;

			uint32_t widthInTiles = 0, heightInTiles = 0;
			uint32_t width = 0, height = 0, bpp = 0;

			width = marker->width;
			height = marker->height;

			if(writeImage)
			{
				bpp = getFormatBpp(writeImage->format);
			}
			else if(writeMSAAimage)
			{
				bpp = getFormatBpp(writeMSAAimage->format);
			}

			if(bpp == 64)
			{
				tileSizeH >>= 1;
			}

			if(performResolve || writeMSAAimage || writeMSAAdepthStencilImage)
			{
				tileSizeW >>= 1;
				tileSizeH >>= 1;
			}

			widthInTiles = divRoundUp(width, tileSizeW);
			heightInTiles = divRoundUp(height, tileSizeH);

			//pad width if rendering to miplevel
			if(marker->mipLevel > 0)
			{
				width = getPow2Pad(width);
				width = width < 4 ? 4 : width;
			}

			submitCl.max_x_tile = widthInTiles - 1;
			submitCl.max_y_tile = heightInTiles - 1;
			submitCl.width = width;
			submitCl.height = height;
			submitCl.flags |= marker->flags;

			submitCl.bo_handles = getCPAptrFromOffset(cmdbuf->handlesCl.CPA, marker->handlesBufOffset + cmdbuf->handlesCl.offset);
			submitCl.bin_cl = ((uint8_t*)marker) + sizeof(CLMarker);
			submitCl.shader_rec = getCPAptrFromOffset(cmdbuf->shaderRecCl.CPA, marker->shaderRecBufOffset + cmdbuf->shaderRecCl.offset);
			submitCl.uniforms = getCPAptrFromOffset(cmdbuf->uniformsCl.CPA, marker->uniformsBufOffset + cmdbuf->uniformsCl.offset);

			if(marker->perfmonID)
			{
				uint32_t perfmonSelector = 0;
				uint32_t* perfmonIDptr = (uint32_t*)marker->perfmonID;

				if(pSubmits->pNext)
				{
					VkPerformanceQuerySubmitInfoKHR* perfQuerySubmitInfo = pSubmits->pNext;
					perfmonSelector = perfQuerySubmitInfo->counterPassIndex;
				}

				submitCl.perfmonid = *(perfmonIDptr + perfmonSelector);
			}

			//marker not closed yet
			//close here
			if(!marker->size)
			{
				clCloseCurrentMarker(&cmdbuf->binCl, &cmdbuf->handlesCl, &cmdbuf->shaderRecCl, cmdbuf->shaderRecCount, &cmdbuf->uniformsCl);
			}

			submitCl.bo_handle_count = marker->handlesSize / 4;
			submitCl.bin_cl_size = marker->size;
			submitCl.shader_rec_size = marker->shaderRecSize;
			submitCl.shader_rec_count = marker->shaderRecCount;
			submitCl.uniforms_size = marker->uniformsSize;

			/**
			printf("BCL:\n");
			uint8_t* mem = malloc(marker->size);
			memcpy(mem, marker+1, marker->size);
			clDump(mem, marker->size);
			free(mem);

			printf("BO handles: ");
			for(int d = 0; d < marker->handlesSize / 4; ++d)
			{
				printf("%u ", *(((uint32_t*)getCPAptrFromOffset(cmdbuf->handlesCl.CPA, marker->handlesBufOffset + cmdbuf->handlesCl.offset))+d));
			}
			printf("\nUniforms: ");
			for(int d = 0; d < marker->uniformsSize / 4; ++d)
			{
				printf("%i ", *(((uint32_t*)getCPAptrFromOffset(cmdbuf->uniformsCl.CPA, marker->uniformsBufOffset + cmdbuf->uniformsCl.offset))+d));
			}
			printf("\nShader recs: ");
			uint8_t* ptr = getCPAptrFromOffset(cmdbuf->shaderRecCl.CPA, marker->shaderRecBufOffset + cmdbuf->shaderRecCl.offset + (3 + 3) * 4);
			for(int d = 0; d < marker->shaderRecCount; ++d)
			{
				uint8_t flags = *ptr;
				uint8_t fragmentShaderIsSingleThreaded = flags & (1 << 0);
				uint8_t pointSizeIncludedInShadedVertexData = (flags & (1 << 1)) >> 1;
				uint8_t enableClipping = (flags & (1 << 2)) >> 2;
				ptr += 2;

				uint8_t fragmentNumberOfUniforms = *ptr; ptr++;
				uint8_t fragmentNumberOfVaryings = *ptr; ptr++;
				uint32_t fragmentShaderCodeAddress = *(uint32_t*)ptr; ptr+=4;
				uint32_t fragmentShaderUniformAddress = *(uint32_t*)ptr; ptr+=4;

				uint16_t vertexNumberOfUniforms = *(uint16_t*)ptr; ptr+=2;
				uint8_t vertexAttribSelectBits = *ptr; ptr++;
				uint8_t vertexAttribTotalSize = *ptr; ptr++;
				uint32_t vertexShaderCodeAddress = *(uint32_t*)ptr; ptr+=4;
				uint32_t vertexShaderUniformAddress = *(uint32_t*)ptr; ptr+=4;

				uint16_t coordNumberOfUniforms = *(uint16_t*)ptr; ptr+=2;
				uint8_t coordAttribSelectBits = *ptr; ptr++;
				uint8_t coordAttribTotalSize = *ptr; ptr++;
				uint32_t coordShaderCodeAddress = *(uint32_t*)ptr; ptr+=4;
				uint32_t coordShaderUniformAddress = *(uint32_t*)ptr; ptr+=4;

				printf("\nfragmentShaderIsSingleThreaded: %i", fragmentShaderIsSingleThreaded);
				printf("\npointSizeIncludedInShadedVertexData: %i", pointSizeIncludedInShadedVertexData);
				printf("\nenableClipping: %i", enableClipping);

				printf("\nfragmentNumberOfUniforms: %i", fragmentNumberOfUniforms);
				printf("\nfragmentNumberOfVaryings: %i", fragmentNumberOfVaryings);
				printf("\nfragmentShaderCodeAddress: %i", fragmentShaderCodeAddress);
				printf("\nfragmentShaderUniformAddress: %i", fragmentShaderUniformAddress);

				printf("\nvertexNumberOfUniforms: %i", vertexNumberOfUniforms);
				printf("\nvertexAttribSelectBits: %i", vertexAttribSelectBits);
				printf("\nvertexAttribTotalSize: %i", vertexAttribTotalSize);
				printf("\nvertexShaderCodeAddress: %i", vertexShaderCodeAddress);
				printf("\nvertexShaderUniformAddress: %i", vertexShaderUniformAddress);

				printf("\ncoordNumberOfUniforms: %i", coordNumberOfUniforms);
				printf("\ncoordAttribSelectBits: %i", coordAttribSelectBits);
				printf("\ncoordAttribTotalSize: %i", coordAttribTotalSize);
				printf("\ncoordShaderCodeAddress: %i", coordShaderCodeAddress);
				printf("\ncoordShaderUniformAddress: %i", coordShaderUniformAddress);

				uint8_t numAttribs = 0;
				for(uint8_t e = 0; e < 8; ++e)
				{
					numAttribs += (vertexAttribSelectBits & (1 << e)) >> e;
				}

				printf("\nnumattribs: %i", numAttribs);
				for(uint8_t e = 0; e < numAttribs; ++e)
				{
					uint32_t attribBaseAddress = *(uint32_t*)ptr; ptr+=4;
					uint8_t attribNumBytes = *ptr; ptr++;
					uint8_t attribStride = *ptr; ptr++;
					uint8_t attribVsVPMOffset = *ptr; ptr++;
					uint8_t attribCsVPMOffset = *ptr; ptr++;

					printf("\nattrib \#%i", e);
					printf("\nattribBaseAddress: %i", attribBaseAddress);
					printf("\nattribNumBytes: %i", attribNumBytes);
					printf("\nattribStride: %i", attribStride);
					printf("\nattribVsVPMOffset: %i", attribVsVPMOffset);
					printf("\nattribCsVPMOffset: %i", attribCsVPMOffset);
				}
			}
			printf("\nwidth height: %u, %u\n", submitCl.width, submitCl.height);
			printf("tile min/max: %u,%u %u,%u\n", submitCl.min_x_tile, submitCl.min_y_tile, submitCl.max_x_tile, submitCl.max_y_tile);
			printf("color read surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.color_read.hindex, submitCl.color_read.offset, submitCl.color_read.bits, submitCl.color_read.flags);
			printf("color write surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.color_write.hindex, submitCl.color_write.offset, submitCl.color_write.bits, submitCl.color_write.flags);
			printf("zs read surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.zs_read.hindex, submitCl.zs_read.offset, submitCl.zs_read.bits, submitCl.zs_read.flags);
			printf("zs write surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.zs_write.hindex, submitCl.zs_write.offset, submitCl.zs_write.bits, submitCl.zs_write.flags);
			printf("msaa color write surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.msaa_color_write.hindex, submitCl.msaa_color_write.offset, submitCl.msaa_color_write.bits, submitCl.msaa_color_write.flags);
			printf("msaa zs write surf: hindex, offset, bits, flags %u %u %u %u\n", submitCl.msaa_zs_write.hindex, submitCl.msaa_zs_write.offset, submitCl.msaa_zs_write.bits, submitCl.msaa_zs_write.flags);
			printf("clear color packed rgba %u %u\n", submitCl.clear_color[0], submitCl.clear_color[1]);
			printf("clear z %u\n", submitCl.clear_z);
			printf("clear s %u\n", submitCl.clear_s);
			printf("flags %u\n", submitCl.flags);
			printf("perfmonID %u\n", submitCl.perfmonid);
			/**/

			assert(submitCl.bo_handle_count > 0);

			//TODO
			while(lastSeqnoGuard);
			{
				lastSeqnoGuard = 1;
				//submit ioctl
				vc4_cl_submit(controlFd, &submitCl, &queue->lastEmitSeqno, &lastFinishedSeqno);
				lastSeqnoGuard = 0;
			}

			//advance in linked list
			marker = marker->nextMarkerOffset == -1 ? 0 : getCPAptrFromOffset(cmdbuf->binCl.CPA, marker->nextMarkerOffset + cmdbuf->binCl.offset);
		}
	}

	for(int c = 0; c < pSubmits->commandBufferCount; ++c)
	{
		if(pSubmits->pCommandBuffers[c]->state == CMDBUF_STATE_PENDING)
		{
			if(pSubmits->pCommandBuffers[c]->usageFlags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
			{
				pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_INVALID;
			}
			else
			{
				pSubmits->pCommandBuffers[c]->state = CMDBUF_STATE_EXECUTABLE;
			}
		}
	}

	for(int c = 0; c < pSubmits->signalSemaphoreCount; ++c)
	{
		sem_post((sem_t*)pSubmits->pSignalSemaphores[c]);
	}

	_fence* f = fence;
	if(f)
	{
		f->seqno = queue->lastEmitSeqno;
	}

	PROFILEEND(RPIFUNC(vkQueueSubmit));
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkFreeCommandBuffers
 * Any primary command buffer that is in the recording or executable state and has any element of pCommandBuffers recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkFreeCommandBuffers)(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		uint32_t                                    commandBufferCount,
		const VkCommandBuffer*                      pCommandBuffers)
{
	PROFILESTART(RPIFUNC(vkFreeCommandBuffers));

	assert(device);
	assert(commandPool);
	assert(pCommandBuffers);

	_commandPool* cp = (_commandPool*)commandPool;

	for(int c = 0; c < commandBufferCount; ++c)
	{
		if(pCommandBuffers[c])
		{
			consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->binCl.offset), pCommandBuffers[c]->binCl.numBlocks);
			consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->handlesCl.offset), pCommandBuffers[c]->handlesCl.numBlocks);
			consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->shaderRecCl.offset), pCommandBuffers[c]->shaderRecCl.numBlocks);
			consecutivePoolFree(&cp->cpa, getCPAptrFromOffset(&cp->cpa, pCommandBuffers[c]->uniformsCl.offset), pCommandBuffers[c]->uniformsCl.numBlocks);
			poolFree(&cp->pa, pCommandBuffers[c]);
		}
	}

	PROFILEEND(RPIFUNC(vkFreeCommandBuffers));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyCommandPool
 * When a pool is destroyed, all command buffers allocated from the pool are freed.
 * Any primary command buffer allocated from another VkCommandPool that is in the recording or executable state and has a secondary command buffer
 * allocated from commandPool recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyCommandPool)(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyCommandPool));

	assert(device);

	_commandPool* cp = (_commandPool*)commandPool;

	if(cp)
	{
		FREE(cp->pa.buf);
		FREE(cp->cpa.buf);
		destroyPoolAllocator(&cp->pa);
		destroyConsecutivePoolAllocator(&cp->cpa);
		FREE(cp);
	}

	PROFILEEND(RPIFUNC(vkDestroyCommandPool));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkTrimCommandPool
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkTrimCommandPool)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolTrimFlags                      flags)
{
	PROFILESTART(RPIFUNC(vkTrimCommandPool));

	assert(device);
	assert(commandPool);

	_commandPool* cp = commandPool;

	//TODO trim cp's pool allocator and consecutive pool allocator
	//by reallocating to just used size
	//kinda silly, as if you need memory afterwards we need to reallocate again...

	PROFILEEND(RPIFUNC(vkTrimCommandPool));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkResetCommandPool
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetCommandPool)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolResetFlags                     flags)
{
	PROFILESTART(RPIFUNC(vkResetCommandPool));

	assert(device);
	assert(commandPool);

	_commandPool* cp = commandPool;

	for(char* c = cp->pa.buf; c != cp->pa.buf + cp->pa.size; c += cp->pa.blockSize)
	{
		char* d = cp->pa.nextFreeBlock;
		while(d)
		{
			if(c == d) break;

			d = *(uint32_t*)d;
		}

		if(c == d) //block is free, as we found it in the free chain
		{
			continue;
		}
		else
		{
			//we found a valid block
			_commandBuffer* cb = c;
			assert(cb->state != CMDBUF_STATE_PENDING);
			cb->state = CMDBUF_STATE_INITIAL;
		}
	}

	//TODO secondary command buffers

	//TODO reset flag --> free all pool resources

	PROFILEEND(RPIFUNC(vkResetCommandPool));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkResetCommandBuffer
 */
VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetCommandBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkCommandBufferResetFlags                   flags)
{
	PROFILESTART(RPIFUNC(vkResetCommandBuffer));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	assert(cb->state != CMDBUF_STATE_PENDING);

	assert(cb->cp->resetAble);

	if(cb->state == CMDBUF_STATE_RECORDING || cb->state == CMDBUF_STATE_EXECUTABLE)
	{
		cb->state = CMDBUF_STATE_INVALID;
	}
	else
	{
		cb->state = CMDBUF_STATE_INITIAL;
	}

	if(flags & VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)
	{
		//TODO release resources
	}

	//reset commandbuffer state
	commandBuffer->shaderRecCount = 0;

	//preserve allocated blocks, only free them if the app requests so
	commandBuffer->binCl.nextFreeByteOffset = commandBuffer->binCl.offset;
	commandBuffer->binCl.currMarkerOffset = -1;

	commandBuffer->handlesCl.nextFreeByteOffset = commandBuffer->handlesCl.offset;
	commandBuffer->handlesCl.currMarkerOffset = -1;

	commandBuffer->shaderRecCl.nextFreeByteOffset = commandBuffer->shaderRecCl.offset;
	commandBuffer->shaderRecCl.currMarkerOffset = -1;

	commandBuffer->uniformsCl.nextFreeByteOffset = commandBuffer->uniformsCl.offset;
	commandBuffer->uniformsCl.currMarkerOffset = -1;

	commandBuffer->graphicsPipeline = 0;
	commandBuffer->computePipeline = 0;
	commandBuffer->numDrawCallsSubmitted = 0;
	commandBuffer->indexBuffer = 0;
	commandBuffer->indexBufferOffset = 0;
	commandBuffer->vertexBufferDirty = 1;
	commandBuffer->indexBufferDirty = 1;
	commandBuffer->viewportDirty = 1;
	commandBuffer->lineWidthDirty = 1;
	commandBuffer->depthBiasDirty = 1;
	commandBuffer->graphicsPipelineDirty = 1;
	commandBuffer->computePipelineDirty = 1;
	commandBuffer->subpassDirty = 1;
	commandBuffer->blendConstantsDirty = 1;
	commandBuffer->scissorDirty = 1;
	commandBuffer->depthBoundsDirty = 1;
	commandBuffer->stencilCompareMaskDirty = 1;
	commandBuffer->stencilWriteMaskDirty = 1;
	commandBuffer->stencilReferenceDirty = 1;
	commandBuffer->descriptorSetDirty = 1;
	commandBuffer->pushConstantDirty = 1;
	commandBuffer->currRenderPass = 0;

	commandBuffer->perfmonID = 0;

	PROFILEEND(RPIFUNC(vkResetCommandBuffer));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdExecuteCommands)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers)
{
	PROFILESTART(RPIFUNC(vkCmdExecuteCommands));

	PROFILEEND(RPIFUNC(vkCmdExecuteCommands));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDeviceMask)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    deviceMask)
{
	UNSUPPORTED(vkCmdSetDeviceMask);
}
