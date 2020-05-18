#include "common.h"

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSampler(
	VkDevice                                    device,
	const VkSamplerCreateInfo*                  pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSampler*                                  pSampler)
{
	PROFILESTART(rpi_vkCreateSampler);

	assert(device);
	assert(pCreateInfo);
	assert(pSampler);

	_sampler* s = ALLOCATE(sizeof(_sampler), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!s)
	{
		PROFILEEND(rpi_vkCreateSampler);
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
	s->disableAutoLod = s->mipLodBias > 0.0f;

	*pSampler = s;

	PROFILEEND(rpi_vkCreateSampler);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySampler(
	VkDevice                                    device,
	VkSampler                                   sampler,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(rpi_vkDestroySampler);

	assert(device);

	FREE(sampler);

	PROFILEEND(rpi_vkDestroySampler);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSamplerYcbcrConversion(
	VkDevice                                    device,
	const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
	PROFILESTART(rpi_vkCreateSamplerYcbcrConversion);

	//TODO

	PROFILEEND(rpi_vkCreateSamplerYcbcrConversion);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySamplerYcbcrConversion(
	VkDevice                                    device,
	VkSamplerYcbcrConversion                    ycbcrConversion,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(rpi_vkDestroySamplerYcbcrConversion);

	//TODO

	PROFILEEND(rpi_vkDestroySamplerYcbcrConversion);
}
