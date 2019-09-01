#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetViewport
 */
void vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	assert(commandBuffer);
	assert(firstViewport == 0);
	assert(viewportCount == 1);
	assert(pViewports);

	//only 1 viewport is supported

	_commandBuffer* cb = commandBuffer;
	cb->viewport = pViewports[0];

	cb->viewportDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetScissor
 */
void vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	assert(commandBuffer);
	assert(firstScissor == 0);
	assert(scissorCount == 1);
	assert(pScissors);

	//only 1 scissor supported

	_commandBuffer* cb = commandBuffer;
	cb->scissor = pScissors[0];

	cb->scissorDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindVertexBuffers
 */
void vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	for(int c = 0; c < bindingCount; ++c)
	{
		cb->vertexBuffers[firstBinding + c] = pBuffers[c];
		cb->vertexBufferOffsets[firstBinding + c] = pOffsets[c];
	}

	cb->vertexBufferDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearColorImage
 * Color and depth/stencil images can be cleared outside a render pass instance using vkCmdClearColorImage or vkCmdClearDepthStencilImage, respectively.
 * These commands are only allowed outside of a render pass instance.
 */
VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(
		VkCommandBuffer                             commandBuffer,
		VkImage                                     image,
		VkImageLayout                               imageLayout,
		const VkClearColorValue*                    pColor,
		uint32_t                                    rangeCount,
		const VkImageSubresourceRange*              pRanges)
{
	assert(commandBuffer);
	assert(image);
	assert(pColor);

	//TODO this should only flag an image for clearing. This can only be called outside a renderpass
	//actual clearing would only happen:
	// -if image is rendered to (insert clear before first draw call)
	// -if the image is bound for sampling (submit a CL with a clear)
	// -if a command buffer is submitted without any rendering (insert clear)
	// -etc.
	//we shouldn't clear an image if noone uses it

	//TODO ranges support

	assert(imageLayout == VK_IMAGE_LAYOUT_GENERAL ||
		   imageLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR ||
		   imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	assert(commandBuffer->state	 == CMDBUF_STATE_RECORDING);
	assert(_queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT || _queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT);

	_image* i = image;

	assert(i->usageBits & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	//TODO externally sync cmdbuf, cmdpool

	//i->needToClear = 1;
	//i->clearColor[0] = i->clearColor[1] = packVec4IntoABGR8(pColor->float32);


	{ //Simplest case: just submit a job to clear the image
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

		//Reset the current compressed primitives format.  This gets modified
		//by VC4_PACKET_GL_INDEXED_PRIMITIVE and
		//VC4_PACKET_GL_ARRAY_PRIMITIVE, so it needs to be reset at the start
		//of every tile.
		//clFit(commandBuffer, &commandBuffer->binCl, V3D21_PRIMITIVE_LIST_FORMAT_length);
		//clInsertPrimitiveListFormat(&commandBuffer->binCl,
		//							1, //16 bit
		//							2); //tris

		clFit(commandBuffer, &commandBuffer->binCl, V3D21_INCREMENT_SEMAPHORE_length);
		clInsertIncrementSemaphore(&commandBuffer->binCl);
		clFit(commandBuffer, &commandBuffer->binCl, V3D21_FLUSH_length);
		clInsertFlush(&commandBuffer->binCl);
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearDepthStencilImage
 */
VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearDepthStencilValue*             pDepthStencil,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges)
{
	assert(commandBuffer);
	assert(image);
	assert(pDepthStencil);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearAttachments
 */
VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    attachmentCount,
	const VkClearAttachment*                    pAttachments,
	uint32_t                                    rectCount,
	const VkClearRect*                          pRects)
{
	assert(commandBuffer);
	assert(pAttachments);
	assert(pRects);

	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdFillBuffer
 */
VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                size,
	uint32_t                                    data)
{
	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdUpdateBuffer
 */
VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                dataSize,
	const void*                                 pData)
{
	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindIndexBuffer
 */
VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	VkIndexType                                 indexType)
{
	//TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetLineWidth
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(
	VkCommandBuffer                             commandBuffer,
	float                                       lineWidth)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->lineWidth = lineWidth;

	cb->lineWidthDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetDepthBias
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(
	VkCommandBuffer                             commandBuffer,
	float                                       depthBiasConstantFactor,
	float                                       depthBiasClamp,
	float                                       depthBiasSlopeFactor)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->depthBiasConstantFactor = depthBiasConstantFactor;
	cb->depthBiasClamp = depthBiasClamp;
	cb->depthBiasSlopeFactor = depthBiasSlopeFactor;

	cb->depthBiasDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetBlendConstants
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(
	VkCommandBuffer                             commandBuffer,
	const float                                 blendConstants[4])
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	memcpy(cb->blendConstants, blendConstants, 4 * sizeof(float));

	cb->blendConstantsDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetDepthBounds
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(
	VkCommandBuffer                             commandBuffer,
	float                                       minDepthBounds,
	float                                       maxDepthBounds)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->minDepthBounds = minDepthBounds;
	cb->maxDepthBounds = maxDepthBounds;

	cb->depthBoundsDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilCompareMask
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    compareMask)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilCompareMask[0] = compareMask;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilCompareMask[1] = compareMask;
	}

	cb->stencilCompareMaskDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilWriteMask
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    writeMask)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilWriteMask[0] = writeMask;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilWriteMask[1] = writeMask;
	}

	cb->stencilWriteMaskDirty = 1;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilReference
 */
VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    reference)
{
	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilReference[0] = reference;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilReference[1] = reference;
	}

	cb->stencilReferenceDirty = 1;
}
