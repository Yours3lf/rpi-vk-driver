#include "common.h"

#include "kernel/vc4_packet.h"
#include "../brcm/cle/v3d_decoder.h"
#include "../brcm/clif/clif_dump.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffers-pools
 * Command pools are opaque objects that command buffer memory is allocated from, and which allow the implementation to amortize the
 * cost of resource creation across multiple command buffers. Command pools are externally synchronized, meaning that a command pool must
 * not be used concurrently in multiple threads. That includes use via recording commands on any command buffers allocated from the pool,
 * as well as operations that allocate, free, and reset command buffers or the pool itself.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(
		VkDevice                                    device,
		const VkCommandPoolCreateInfo*              pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkCommandPool*                              pCommandPool)
{
	assert(device);
	assert(pCreateInfo);

	//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	//specifies that command buffers allocated from the pool will be short-lived, meaning that they will be reset or freed in a relatively short timeframe.
	//This flag may be used by the implementation to control memory allocation behavior within the pool.
	//--> definitely use pool allocator

	//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	//allows any command buffer allocated from a pool to be individually reset to the initial state; either by calling vkResetCommandBuffer, or via the implicit reset when calling vkBeginCommandBuffer.
	//If this flag is not set on a pool, then vkResetCommandBuffer must not be called for any command buffer allocated from that pool.

	//TODO pool family ignored for now

	_commandPool* cp = ALLOCATE(sizeof(_commandPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!cp)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	cp->queueFamilyIndex = pCreateInfo->queueFamilyIndex;

	//TODO CTS fails as we can't allocate enough memory for some reason
	//tweak system allocation as root using:
	//make sure kernel denies memory allocation that it won't be able to serve
	//sysctl -w vm.overcommit_memory="2"
	//specify after how much memory used the kernel will start denying requests
	//sysctl -w vm.overcommit_ratio="80"
	//



	//initial number of command buffers to hold
	int numCommandBufs = 128;
	int consecutivePoolSize = ARM_PAGE_SIZE*128;
	int consecutiveBlockSize = ARM_PAGE_SIZE>>2;

	static int counter = 0;

	//if(pCreateInfo->flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	{
		//use pool allocator
		void* pamem = ALLOCATE(numCommandBufs * sizeof(_commandBuffer), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!pamem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->pa = createPoolAllocator(pamem, sizeof(_commandBuffer), numCommandBufs * sizeof(_commandBuffer));

		void* cpamem = ALLOCATE(consecutivePoolSize, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!cpamem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		cp->cpa = createConsecutivePoolAllocator(cpamem, consecutiveBlockSize, consecutivePoolSize);
	}

	*pCommandPool = (VkCommandPool)cp;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffer-allocation
 * vkAllocateCommandBuffers can be used to create multiple command buffers. If the creation of any of those command buffers fails,
 * the implementation must destroy all successfully created command buffer objects from this command, set all entries of the pCommandBuffers array to NULL and return the error.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(
		VkDevice                                    device,
		const VkCommandBufferAllocateInfo*          pAllocateInfo,
		VkCommandBuffer*                            pCommandBuffers)
{
	assert(device);
	assert(pAllocateInfo);
	assert(pCommandBuffers);

	VkResult res = VK_SUCCESS;

	_commandPool* cp = (_commandPool*)pAllocateInfo->commandPool;

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

			pCommandBuffers[c]->dev = device;

			pCommandBuffers[c]->shaderRecCount = 0;
			pCommandBuffers[c]->usageFlags = 0;
			pCommandBuffers[c]->state = CMDBUF_STATE_INITIAL;
			pCommandBuffers[c]->cp = cp;
			clInit(&pCommandBuffers[c]->binCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->handlesCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->shaderRecCl, consecutivePoolAllocate(&cp->cpa, 1));
			clInit(&pCommandBuffers[c]->uniformsCl, consecutivePoolAllocate(&cp->cpa, 1));

			pCommandBuffers[c]->renderpass = 0;
			pCommandBuffers[c]->fbo = 0;
			pCommandBuffers[c]->currentSubpass = 0;
			pCommandBuffers[c]->graphicsPipeline = 0;
			pCommandBuffers[c]->computePipeline = 0;
			pCommandBuffers[c]->numDrawCallsSubmitted = 0;
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

			if(!pCommandBuffers[c]->binCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->handlesCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->shaderRecCl.buffer)
			{
				res = VK_ERROR_OUT_OF_HOST_MEMORY;
				break;
			}

			if(!pCommandBuffers[c]->uniformsCl.buffer)
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
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->binCl, pCommandBuffers[c]->binCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->handlesCl, pCommandBuffers[c]->handlesCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->shaderRecCl, pCommandBuffers[c]->shaderRecCl.numBlocks);
				consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->uniformsCl, pCommandBuffers[c]->uniformsCl.numBlocks);
				poolFree(&cp->pa, pCommandBuffers[c]);
				pCommandBuffers[c] = 0;
			}
		}
	}

	return res;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBeginCommandBuffer
 */
VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(
		VkCommandBuffer                             commandBuffer,
		const VkCommandBufferBeginInfo*             pBeginInfo)
{
	assert(commandBuffer);
	assert(pBeginInfo);

	//TODO

	//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	//specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission.

	//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
	//specifies that a secondary command buffer is considered to be entirely inside a render pass. If this is a primary command buffer, then this bit is ignored

	//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
	//specifies that a command buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary command buffers

	//When a command buffer begins recording, all state in that command buffer is undefined

	commandBuffer->usageFlags = pBeginInfo->flags;
	commandBuffer->state = CMDBUF_STATE_RECORDING;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEndCommandBuffer
 * If there was an error during recording, the application will be notified by an unsuccessful return code returned by vkEndCommandBuffer.
 * If the application wishes to further use the command buffer, the command buffer must be reset. The command buffer must have been in the recording state,
 * and is moved to the executable state.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(
		VkCommandBuffer                             commandBuffer)
{
	assert(commandBuffer);

	commandBuffer->state = CMDBUF_STATE_EXECUTABLE;

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
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(
		VkQueue                                     queue,
		uint32_t                                    submitCount,
		const VkSubmitInfo*                         pSubmits,
		VkFence                                     fence)
{
	assert(queue);

	for(int c = 0; c < pSubmits->waitSemaphoreCount; ++c)
	{
		sem_wait((sem_t*)pSubmits->pWaitSemaphores[c]);
	}

	//TODO: deal with pSubmits->pWaitDstStageMask

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

		//first entry is assumed to be a marker
		CLMarker* marker = cmdbuf->binCl.buffer;

		//a command buffer may contain multiple render passes
		//and commands outside render passes such as clear commands
		//each of these corresponds to a control list submit

		//submit each separate control list
		while(marker)
		{
			struct drm_vc4_submit_cl submitCl =
			{
				.color_read.hindex = ~0,
				.zs_read.hindex = ~0,
				.color_write.hindex = ~0,
				.msaa_color_write.hindex = ~0,
				.zs_write.hindex = ~0,
				.msaa_zs_write.hindex = ~0,
			};

			_image* i = marker->image;

			//This should not result in an insertion!
			uint32_t imageIdx = clGetHandleIndex(&cmdbuf->handlesCl, marker->handlesBuf, marker->handlesSize, i->boundMem->bo);

			//fill out submit cl fields
			submitCl.color_write.hindex = imageIdx;
			submitCl.color_write.offset = 0;
			submitCl.color_write.flags = 0;
			submitCl.color_write.bits =
					VC4_SET_FIELD(getRenderTargetFormatVC4(i->format), VC4_RENDER_CONFIG_FORMAT) |
					VC4_SET_FIELD(i->tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);

			submitCl.clear_color[0] = i->clearColor[0];
			submitCl.clear_color[1] = i->clearColor[1];

			submitCl.min_x_tile = 0;
			submitCl.min_y_tile = 0;

			uint32_t tileSizeW = 64;
			uint32_t tileSizeH = 64;

			if(i->samples > 1)
			{
				tileSizeW >>= 1;
				tileSizeH >>= 1;
			}

			if(getFormatBpp(i->format) == 64)
			{
				tileSizeH >>= 1;
			}

			uint32_t widthInTiles = divRoundUp(i->width, tileSizeW);
			uint32_t heightInTiles = divRoundUp(i->height, tileSizeH);

			submitCl.max_x_tile = widthInTiles - 1;
			submitCl.max_y_tile = heightInTiles - 1;
			submitCl.width = i->width;
			submitCl.height = i->height;
			submitCl.flags |= marker->flags;
			submitCl.clear_z = 0; //TODO
			submitCl.clear_s = 0;

			submitCl.bo_handles = marker->handlesBuf;
			submitCl.bin_cl = ((uint8_t*)marker) + sizeof(CLMarker);
			submitCl.shader_rec = marker->shaderRecBuf;
			submitCl.uniforms = marker->uniformsBuf;

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

			/**/
			printf("BCL:\n");
			clDump(((uint8_t*)marker) + sizeof(CLMarker), marker->size);
			printf("BO handles: ");
			for(int d = 0; d < marker->handlesSize / 4; ++d)
			{
				printf("%u ", *((uint32_t*)(marker->handlesBuf)+d));
			}
			printf("\nUniforms: ");
			for(int d = 0; d < marker->uniformsSize / 4; ++d)
			{
				printf("%u ", *((uint32_t*)(marker->uniformsBuf)+d));
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
			/**/


			//submit ioctl
			static uint64_t lastFinishedSeqno = 0;
			vc4_cl_submit(controlFd, &submitCl, &queue->lastEmitSeqno, &lastFinishedSeqno);

			//advance in linked list
			marker = marker->nextMarker;
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

	//TODO is this correct?
	_fence* f = fence;
	if(f)
	{
		f->seqno = queue->lastEmitSeqno;
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkFreeCommandBuffers
 * Any primary command buffer that is in the recording or executable state and has any element of pCommandBuffers recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		uint32_t                                    commandBufferCount,
		const VkCommandBuffer*                      pCommandBuffers)
{
	assert(device);
	assert(commandPool);
	assert(pCommandBuffers);

	_commandPool* cp = (_commandPool*)commandPool;

	for(int c = 0; c < commandBufferCount; ++c)
	{
		if(pCommandBuffers[c])
		{
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->binCl, pCommandBuffers[c]->binCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->handlesCl, pCommandBuffers[c]->handlesCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->shaderRecCl, pCommandBuffers[c]->shaderRecCl.numBlocks);
			consecutivePoolFree(&cp->cpa, &pCommandBuffers[c]->uniformsCl, pCommandBuffers[c]->uniformsCl.numBlocks);
			poolFree(&cp->pa, pCommandBuffers[c]);
		}
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyCommandPool
 * When a pool is destroyed, all command buffers allocated from the pool are freed.
 * Any primary command buffer allocated from another VkCommandPool that is in the recording or executable state and has a secondary command buffer
 * allocated from commandPool recorded into it, becomes invalid.
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(
		VkDevice                                    device,
		VkCommandPool                               commandPool,
		const VkAllocationCallbacks*                pAllocator)
{
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
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkTrimCommandPool
 */
VKAPI_ATTR void VKAPI_CALL vkTrimCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolTrimFlags                      flags)
{
	assert(device);
	assert(commandPool);

	_commandPool* cp = commandPool;

	//TODO??
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkResetCommandPool
 */
VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolResetFlags                     flags)
{
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

	//TODO secondary command buffer stuff
	//TODO reset flag
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkResetCommandBuffer
 */
VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(
	VkCommandBuffer                             commandBuffer,
	VkCommandBufferResetFlags                   flags)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	assert(cb->state != CMDBUF_STATE_PENDING);

	if(cb->state == CMDBUF_STATE_RECORDING || cb->state == CMDBUF_STATE_EXECUTABLE)
	{
		cb->state = CMDBUF_STATE_INVALID;
	}
	else
	{
		cb->state = CMDBUF_STATE_INITIAL;
	}

	//TODO flag?
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMask(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    deviceMask)
{

}
