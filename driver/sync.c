#include "common.h"

#include "kernel/vc4_packet.h"

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

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//we'll probably just use an IOCTL to wait for a GPU sequence number to complete.
	sem_t* s = malloc(sizeof(sem_t));
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

		assert(i->layout == pImageMemoryBarriers[c].oldLayout || i->layout == VK_IMAGE_LAYOUT_UNDEFINED);

		if(srcStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT &&
		   pImageMemoryBarriers[c].srcAccessMask & VK_ACCESS_TRANSFER_WRITE_BIT &&
		   i->needToClear)
		{
			//insert CRs to clear the image

			assert(i->layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			clFit(commandBuffer, &commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
			clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
												 0, 0, 0, 0,
												 getFormatBpp(i->format) == 64, //64 bit color mode
												 i->samples > 1, //msaa
												 i->width, i->height, 0, 0, 0);

			//START_TILE_BINNING resets the statechange counters in the hardware,
			//which are what is used when a primitive is binned to a tile to
			//figure out what new state packets need to be written to that tile's
			//command list.
			clFit(commandBuffer, &commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
			clInsertStartTileBinning(&commandBuffer->binCl);

			//Reset the current compressed primitives format.  This gets modified
			//by VC4_PACKET_GL_INDEXED_PRIMITIVE and
			//VC4_PACKET_GL_ARRAY_PRIMITIVE, so it needs to be reset at the start
			//of every tile.
			clFit(commandBuffer, &commandBuffer->binCl, V3D21_PRIMITIVE_LIST_FORMAT_length);
			clInsertPrimitiveListFormat(&commandBuffer->binCl,
										1, //16 bit
										2); //tris

			clFit(commandBuffer, &commandBuffer->handlesCl, 4);
			uint32_t idx = clGetHandleIndex(&commandBuffer->handlesCl, i->handle);
			commandBuffer->submitCl.color_write.hindex = idx;
			commandBuffer->submitCl.color_write.offset = 0;
			commandBuffer->submitCl.color_write.flags = 0;
			//TODO format
			commandBuffer->submitCl.color_write.bits =
					VC4_SET_FIELD(VC4_RENDER_CONFIG_FORMAT_RGBA8888, VC4_RENDER_CONFIG_FORMAT) |
					VC4_SET_FIELD(i->tiling, VC4_RENDER_CONFIG_MEMORY_FORMAT);

			commandBuffer->submitCl.clear_color[0] = i->clearColor[0];
			commandBuffer->submitCl.clear_color[1] = i->clearColor[1];

			//TODO ranges
			commandBuffer->submitCl.min_x_tile = 0;
			commandBuffer->submitCl.min_y_tile = 0;

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

			commandBuffer->submitCl.max_x_tile = widthInTiles - 1;
			commandBuffer->submitCl.max_y_tile = heightInTiles - 1;
			commandBuffer->submitCl.width = i->width;
			commandBuffer->submitCl.height = i->height;
			commandBuffer->submitCl.flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;
			commandBuffer->submitCl.clear_z = 0; //TODO
			commandBuffer->submitCl.clear_s = 0;
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
			vc4_seqno_wait(controlFd, &lastFinishedSeqno, device->queues[c][d].lastEmitSeqno, WAIT_TIMEOUT_INFINITE);
		}
	}

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
	assert(semaphore);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	sem_destroy((sem_t*)semaphore);
}
