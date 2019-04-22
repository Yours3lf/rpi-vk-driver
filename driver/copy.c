#include "common.h"

//TODO

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageBlit*                          pRegions,
	VkFilter                                    filter)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageResolve*                       pRegions)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageCopy*                          pRegions)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferCopy*                         pRegions)
{

}
