#include "common.h"

#include "kernel/vc4_packet.h"

//-----------------------------
//Semaphore vs Fence:
// Semaphore is GPU to GPU sync
// Fence is GPU to CPU sync
// Both are signalled by the GPU
// Both are multi-queue
// But Fence can be waited on by the CPU
// Semaphore can only be waited on by the GPU
//
//Events are general can be signalled by the CPU or the GPU
// But can only be waited on by the GPU
// Limited to a single queue
//
//TODO as a result the current semaphore
//implementation is wrong
//maybe use:
//clInsertWaitOnSemaphore
//clInsertIncrementSemaphore
//
//seems like each binCL needs to end with increment semaphore
//signalling that binning is done
//and each renderCL starts with a wait semaphore (to wait for binning)
//
//in theory we could add a wait for semaphore to the start of a binCL
//and an increment semaphore to either to the end of another binCL or renderCL
//but we can't control renderCLs as the kernel side creates those...
//
//also there's only one of this semaphore, and in Vulkan you can have many
//and should only signal those selected
//so maybe we could emulate this in shaders?
//ie. stall shader until a value is something?
//and increment said value?
//but we'd need to patch shaders and it'd probably be slow...
//-----------------------------

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateSemaphore
 * Semaphores are a synchronization primitive that can be used to insert a dependency between batches submitted to queues.
 * Semaphores have two states - signaled and unsignaled. The state of a semaphore can be signaled after execution of a batch of commands is completed.
 * A batch can wait for a semaphore to become signaled before it begins execution, and the semaphore is also unsignaled before the batch begins execution.
 * As with most objects in Vulkan, semaphores are an interface to internal data which is typically opaque to applications.
 * This internal data is referred to as a semaphore’s payload. However, in order to enable communication with agents outside of the current device,
 * it is necessary to be able to export that payload to a commonly understood format, and subsequently import from that format as well.
 * The internal data of a semaphore may include a reference to any resources and pending work associated with signal or unsignal operations performed on that semaphore object.
 * Mechanisms to import and export that internal data to and from semaphores are provided below.
 * These mechanisms indirectly enable applications to share semaphore state between two or more semaphores and other synchronization primitives across process and API boundaries.
 * When created, the semaphore is in the unsignaled state.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(
		VkDevice                                    device,
		const VkSemaphoreCreateInfo*                pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSemaphore*                                pSemaphore)
{
	assert(device);
	assert(pSemaphore);

	//we'll probably just use an IOCTL to wait for a GPU sequence number to complete.
	sem_t* s = ALLOCATE(sizeof(sem_t), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!s)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}
	sem_init(s, 0, 0); //create semaphore unsignalled, shared between threads

	*pSemaphore = (VkSemaphore)s;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdPipelineBarrier
 * vkCmdPipelineBarrier is a synchronization command that inserts a dependency between commands submitted to the same queue, or between commands in the same subpass.
 * When vkCmdPipelineBarrier is submitted to a queue, it defines a memory dependency between commands that were submitted before it, and those submitted after it.
 * If vkCmdPipelineBarrier was recorded outside a render pass instance, the first synchronization scope includes all commands that occur earlier in submission order.
 * If vkCmdPipelineBarrier was recorded inside a render pass instance, the first synchronization scope includes only commands that occur earlier in submission order within the same subpass.
 * In either case, the first synchronization scope is limited to operations on the pipeline stages determined by the source stage mask specified by srcStageMask.
 *
 * If vkCmdPipelineBarrier was recorded outside a render pass instance, the second synchronization scope includes all commands that occur later in submission order.
 * If vkCmdPipelineBarrier was recorded inside a render pass instance, the second synchronization scope includes only commands that occur later in submission order within the same subpass.
 * In either case, the second synchronization scope is limited to operations on the pipeline stages determined by the destination stage mask specified by dstStageMask.
 *
 * The first access scope is limited to access in the pipeline stages determined by the source stage mask specified by srcStageMask.
 * Within that, the first access scope only includes the first access scopes defined by elements of the pMemoryBarriers,
 * pBufferMemoryBarriers and pImageMemoryBarriers arrays, which each define a set of memory barriers. If no memory barriers are specified,
 * then the first access scope includes no accesses.
 *
 * The second access scope is limited to access in the pipeline stages determined by the destination stage mask specified by dstStageMask.
 * Within that, the second access scope only includes the second access scopes defined by elements of the pMemoryBarriers, pBufferMemoryBarriers and pImageMemoryBarriers arrays,
 * which each define a set of memory barriers. If no memory barriers are specified, then the second access scope includes no accesses.
 *
 * If dependencyFlags includes VK_DEPENDENCY_BY_REGION_BIT, then any dependency between framebuffer-space pipeline stages is framebuffer-local - otherwise it is framebuffer-global.
 */
VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(
		VkCommandBuffer                             commandBuffer,
		VkPipelineStageFlags                        srcStageMask,
		VkPipelineStageFlags                        dstStageMask,
		VkDependencyFlags                           dependencyFlags,
		uint32_t                                    memoryBarrierCount,
		const VkMemoryBarrier*                      pMemoryBarriers,
		uint32_t                                    bufferMemoryBarrierCount,
		const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
		uint32_t                                    imageMemoryBarrierCount,
		const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
	assert(commandBuffer);

	//TODO pipeline stage flags
	//VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
	//VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
	//VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
	//VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
	//VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
	//VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
	//VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
	//VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	//VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
	//VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
	//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	//VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	//VK_PIPELINE_STAGE_TRANSFER_BIT
	//VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
	//VK_PIPELINE_STAGE_HOST_BIT
	//VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
	//VK_PIPELINE_STAGE_ALL_COMMANDS_BIT

	//TODO dependency flags
	//VK_DEPENDENCY_BY_REGION_BIT,
	//VK_DEPENDENCY_DEVICE_GROUP_BIT,
	//VK_DEPENDENCY_VIEW_LOCAL_BIT

	//TODO access flags
	//VK_ACCESS_INDIRECT_COMMAND_READ_BIT
	//VK_ACCESS_INDEX_READ_BIT
	//VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
	//VK_ACCESS_UNIFORM_READ_BIT
	//VK_ACCESS_INPUT_ATTACHMENT_READ_BIT
	//VK_ACCESS_SHADER_READ_BIT
	//VK_ACCESS_SHADER_WRITE_BIT
	//VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
	//VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	//VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
	//VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	//VK_ACCESS_TRANSFER_READ_BIT
	//VK_ACCESS_TRANSFER_WRITE_BIT
	//VK_ACCESS_HOST_READ_BIT
	//VK_ACCESS_HOST_WRITE_BIT
	//VK_ACCESS_MEMORY_READ_BIT
	//VK_ACCESS_MEMORY_WRITE_BIT
	//VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX
	//VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX

	//TODO Layout transition flags
	//VK_IMAGE_LAYOUT_UNDEFINED
	//VK_IMAGE_LAYOUT_GENERAL
	//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
	//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	//VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	//VK_IMAGE_LAYOUT_PREINITIALIZED
	//VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
	//VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
	//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	//VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR

	for(int c = 0; c < memoryBarrierCount; ++c)
	{
		//TODO
	}

	for(int c = 0; c < bufferMemoryBarrierCount; ++c)
	{
		//TODO
	}

	for(int c = 0; c < imageMemoryBarrierCount; ++c)
	{
		_image* i = pImageMemoryBarriers[c].image;

		if(srcStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT &&
		   pImageMemoryBarriers[c].srcAccessMask & VK_ACCESS_TRANSFER_WRITE_BIT &&
		   i->needToClear)
		{
			assert(i->layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}

		//transition to new layout
		i->layout = pImageMemoryBarriers[c].newLayout;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDeviceWaitIdle
 * vkDeviceWaitIdle is equivalent to calling vkQueueWaitIdle for all queues owned by device.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(
		VkDevice									device)
{
	assert(device);

	for(int c = 0; c < numQueueFamilies; ++c)
	{
		for(int d = 0; d < device->numQueues[c]; ++d)
		{
			uint64_t lastFinishedSeqno;
			uint64_t timeout = WAIT_TIMEOUT_INFINITE;
			vc4_seqno_wait(controlFd, &lastFinishedSeqno, device->queues[c][d].lastEmitSeqno, &timeout);
		}
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkQueueWaitIdle
 */
VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(
	VkQueue                                     queue)
{
	assert(queue);

	_queue* q = queue;
	uint64_t lastFinishedSeqno;
	uint64_t timeout = WAIT_TIMEOUT_INFINITE;
	vc4_seqno_wait(controlFd, &lastFinishedSeqno, q->lastEmitSeqno, &timeout);

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroySemaphore
 */
VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(
		VkDevice                                    device,
		VkSemaphore                                 semaphore,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	if(semaphore)
	{
		sem_destroy((sem_t*)semaphore);
		FREE(semaphore);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateFence
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(
	VkDevice                                    device,
	const VkFenceCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFence*                                    pFence)
{
	assert(device);
	assert(pCreateInfo);
	assert(pFence);

	_fence* f = ALLOCATE(sizeof(_fence), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!f)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	f->seqno = 0;
	f->signaled = pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT;

	*pFence = f;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyFence
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyFence(
	VkDevice                                    device,
	VkFence                                     fence,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	if(fence)
	{
		FREE(fence);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetFenceStatus
 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(
	VkDevice                                    device,
	VkFence                                     fence)
{
	assert(device);
	assert(fence);

	//TODO update fence status based on last completed seqno?

	_fence* f = fence;
	return f->signaled ? VK_SUCCESS : VK_NOT_READY;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkResetFences
 */
VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences)
{
	assert(device);
	assert(pFences);
	assert(fenceCount > 0);

	for(uint32_t c = 0; c < fenceCount; ++c)
	{
		_fence* f = pFences[c];
		f->signaled = 0;
		f->seqno = 0;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkWaitForFences
 */
VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences,
	VkBool32                                    waitAll,
	uint64_t                                    timeout)
{
	assert(device);
	assert(pFences);
	assert(fenceCount > 0);

	if(waitAll)
	{
		if(!timeout)
		{
			for(uint32_t c = 0; c < fenceCount; ++c)
			{
				_fence* f = pFences[c];
				if(!f->signaled) //if any unsignaled
				{
					return VK_TIMEOUT;
				}

				return VK_SUCCESS;
			}
		}

		//wait for all to be signaled
		for(uint32_t c = 0; c < fenceCount; ++c)
		{
			_fence* f = pFences[c];
			uint64_t lastFinishedSeqno = 0;
			if(!f->signaled)
			{
				int ret = vc4_seqno_wait(controlFd, &lastFinishedSeqno, f->seqno, &timeout);

				if(ret < 0)
				{
					return VK_TIMEOUT;
				}

				f->signaled = 1;
				f->seqno = 0;
			}
		}
	}
	else
	{
		if(!timeout)
		{
			for(uint32_t c = 0; c < fenceCount; ++c)
			{
				_fence* f = pFences[c];
				if(f->signaled) //if any signaled
				{
					return VK_SUCCESS;
				}

				return VK_TIMEOUT;
			}
		}

		//wait for any to be signaled
		for(uint32_t c = 0; c < fenceCount; ++c)
		{
			_fence* f = pFences[c];
			uint64_t lastFinishedSeqno = 0;
			if(!f->signaled)
			{
				int ret = vc4_seqno_wait(controlFd, &lastFinishedSeqno, f->seqno, &timeout);

				if(ret < 0)
				{
					continue;
				}

				f->signaled = 1;
				f->seqno = 0;
				return VK_SUCCESS;
			}
		}

		return VK_TIMEOUT;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    eventCount,
	const VkEvent*                              pEvents,
	VkPipelineStageFlags                        srcStageMask,
	VkPipelineStageFlags                        dstStageMask,
	uint32_t                                    memoryBarrierCount,
	const VkMemoryBarrier*                      pMemoryBarriers,
	uint32_t                                    bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
	uint32_t                                    imageMemoryBarrierCount,
	const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
	//TODO
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(
	VkDevice                                    device,
	VkEvent                                     event)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(
	VkDevice                                    device,
	VkEvent                                     event,
	const VkAllocationCallbacks*                pAllocator)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask)
{
	//TODO
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(
	VkDevice                                    device,
	const VkEventCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkEvent*                                    pEvent)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(
	VkDevice                                    device,
	VkEvent                                     event)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(
	VkDevice                                    device,
	VkEvent                                     event)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask)
{
	//TODO
}
