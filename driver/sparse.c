#include "common.h"

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceSparseImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkSampleCountFlagBits                       samples,
	VkImageUsageFlags                           usage,
	VkImageTiling                               tiling,
	uint32_t*                                   pPropertyCount,
		VkSparseImageFormatProperties*              pProperties)
{
	UNSUPPORTED(vkGetPhysicalDeviceSparseImageFormatProperties);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSparseMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements*            pSparseMemoryRequirements)
{
	UNSUPPORTED(vkGetImageSparseMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkQueueBindSparse(
	VkQueue                                     queue,
	uint32_t                                    bindInfoCount,
	const VkBindSparseInfo*                     pBindInfo,
	VkFence                                     fence)
{
	UNSUPPORTED(vkQueueBindSparse);
	return UNSUPPORTED_RETURN;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceSparseImageFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties2*             pProperties)
{
	UNSUPPORTED(vkGetPhysicalDeviceSparseImageFormatProperties2);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSparseMemoryRequirements2(
	VkDevice                                    device,
	const VkImageSparseMemoryRequirementsInfo2* pInfo,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
	UNSUPPORTED(vkGetImageSparseMemoryRequirements2);
}
