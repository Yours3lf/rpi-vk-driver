#include "common.h"

//TODO

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkSampleCountFlagBits                       samples,
	VkImageUsageFlags                           usage,
	VkImageTiling                               tiling,
	uint32_t*                                   pPropertyCount,
		VkSparseImageFormatProperties*              pProperties)
{

}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements*            pSparseMemoryRequirements)
{

}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(
	VkQueue                                     queue,
	uint32_t                                    bindInfoCount,
	const VkBindSparseInfo*                     pBindInfo,
	VkFence                                     fence)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties2*             pProperties)
{

}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2(
	VkDevice                                    device,
	const VkImageSparseMemoryRequirementsInfo2* pInfo,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{

}
