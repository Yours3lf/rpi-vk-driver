#include "common.h"

//TODO

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(
	VkDevice                                    device,
	const VkSamplerCreateInfo*                  pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSampler*                                  pSampler)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySampler(
	VkDevice                                    device,
	VkSampler                                   sampler,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversion(
	VkDevice                                    device,
	const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversion(
	VkDevice                                    device,
	VkSamplerYcbcrConversion                    ycbcrConversion,
	const VkAllocationCallbacks*                pAllocator)
{

}
