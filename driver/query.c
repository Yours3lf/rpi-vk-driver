#include "common.h"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(
	VkDevice                                    device,
	const VkQueryPoolCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkQueryPool*                                pQueryPool)
{
	//TODO

	if(pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION)
	{
		UNSUPPORTED(VK_QUERY_TYPE_OCCLUSION);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	const VkAllocationCallbacks*                pAllocator)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query,
	VkQueryControlFlags                         flags)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags)
{
	//TODO
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetQueryPoolResults(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	size_t                                      dataSize,
	void*                                       pData,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlagBits                     pipelineStage,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	//TODO
}
