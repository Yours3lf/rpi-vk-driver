#include "common.h"

//TODO
//compute shaders need kernel support

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateComputePipelines(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkComputePipelineCreateInfo*          pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines)
{
	UNSUPPORTED(rpi_vkCreateComputePipelines);
	return UNSUPPORTED_RETURN;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatchIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset)
{
	UNSUPPORTED(rpi_vkCmdDispatchIndirect);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatch(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ)
{
	UNSUPPORTED(rpi_vkCmdDispatch);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatchBase(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    baseGroupX,
	uint32_t                                    baseGroupY,
	uint32_t                                    baseGroupZ,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ)
{
	UNSUPPORTED(rpi_vkCmdDispatchBase);
}
