#include "common.h"

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSampler(
	VkDevice                                    device,
	const VkSamplerCreateInfo*                  pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSampler*                                  pSampler)
{
	assert(device);
	assert(pCreateInfo);
	assert(pSampler);

	_sampler* s = ALLOCATE(sizeof(_sampler), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!s)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	s->minFilter = pCreateInfo->minFilter;
	s->magFilter = pCreateInfo->magFilter;
	s->mipmapMode = pCreateInfo->mipmapMode;
	s->addressModeU = pCreateInfo->addressModeU;
	s->addressModeV = pCreateInfo->addressModeV;
	s->addressModeW = pCreateInfo->addressModeW;
	s->mipLodBias = pCreateInfo->mipLodBias;
	s->anisotropyEnable = pCreateInfo->anisotropyEnable;
	s->maxAnisotropy = pCreateInfo->maxAnisotropy;
	s->compareEnable = pCreateInfo->compareEnable;
	s->compareOp = pCreateInfo->compareOp;
	s->minLod = pCreateInfo->minLod;
	s->maxLod = pCreateInfo->maxLod;
	s->borderColor = pCreateInfo->borderColor;
	s->unnormalizedCoordinates = pCreateInfo->unnormalizedCoordinates;

	*pSampler = s;

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySampler(
	VkDevice                                    device,
	VkSampler                                   sampler,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	FREE(sampler);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSamplerYcbcrConversion(
	VkDevice                                    device,
	const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySamplerYcbcrConversion(
	VkDevice                                    device,
	VkSamplerYcbcrConversion                    ycbcrConversion,
	const VkAllocationCallbacks*                pAllocator)
{
	//TODO
}
