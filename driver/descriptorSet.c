#include "common.h"

//TODO

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(
	VkDevice                                    device,
	const VkDescriptorPoolCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorPool*                           pDescriptorPool)
{
	assert(device);
	assert(pCreateInfo);

	_descriptorPool* dp = ALLOCATE(sizeof(_descriptorPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dp)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	uint32_t imageDescriptorCount = 0, bufferDescriptorCount = 0, texelBufferDescriptorCount = 0;
	for(uint32_t c = 0; c < pCreateInfo->poolSizeCount; ++c)
	{
		switch(pCreateInfo->pPoolSizes[c].type)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			imageDescriptorCount++;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			bufferDescriptorCount++;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			texelBufferDescriptorCount++;
			break;
		}
	}

	//TODO
	//pCreateInfo->flags

	void* dsmem = ALLOCATE(sizeof(_descriptorSet), pCreateInfo->maxSets, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!dsmem)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	dp->descriptorSetPA = createPoolAllocator(dsmem, sizeof(_descriptorSet), sizeof(_descriptorSet) * pCreateInfo->maxSets);
	dp->imageDescriptorCPA = 0;
	dp->bufferDescriptorCPA = 0;
	dp->texelBufferDescriptorCPA = 0;

	if(imageDescriptorCount > 0)
	{
		dp->imageDescriptorCPA = ALLOCATE(sizeof(ConsecutivePoolAllocator), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!dp)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		void* mem = ALLOCATE(sizeof(_descriptorImage), imageDescriptorCount, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		*dp->imageDescriptorCPA = createConsecutivePoolAllocator(mem, sizeof(_descriptorImage), sizeof(_descriptorImage) * imageDescriptorCount);
	}

	if(bufferDescriptorCount > 0)
	{
		dp->bufferDescriptorCPA = ALLOCATE(sizeof(ConsecutivePoolAllocator), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!dp)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		void* mem = ALLOCATE(sizeof(_descriptorBuffer), bufferDescriptorCount, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		*dp->bufferDescriptorCPA = createConsecutivePoolAllocator(mem, sizeof(_descriptorBuffer), sizeof(_descriptorBuffer) * bufferDescriptorCount);
	}

	if(texelBufferDescriptorCount > 0)
	{
		dp->texelBufferDescriptorCPA = ALLOCATE(sizeof(ConsecutivePoolAllocator), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!dp)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		void* mem = ALLOCATE(sizeof(_descriptorTexelBuffer), texelBufferDescriptorCount, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		*dp->texelBufferDescriptorCPA = createConsecutivePoolAllocator(mem, sizeof(_descriptorTexelBuffer), sizeof(_descriptorTexelBuffer) * texelBufferDescriptorCount);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(
	VkDevice                                    device,
	const VkDescriptorSetAllocateInfo*          pAllocateInfo,
	VkDescriptorSet*                            pDescriptorSets)
{
	assert(device);

	_descriptorPool* dp = pAllocateInfo->descriptorPool;

	for(uint32_t c = 0; c < pAllocateInfo->descriptorSetCount; ++c)
	{
		_descriptorSet* ds = poolAllocate(&dp->descriptorSetPA);
		pDescriptorSets[c] = ds;

		_descriptorSetLayout* dsl = pAllocateInfo->pSetLayouts[c];

		//TODO flags

		uint32_t imageDescriptorCount = 0, bufferDescriptorCount = 0, texelBufferDescriptorCount = 0;
		for(uint32_t d = 0; d < dsl->bindingsCount; ++d)
		{
			switch(dsl->bindings[d].descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				imageDescriptorCount++;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				bufferDescriptorCount++;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				texelBufferDescriptorCount++;
				break;
			}
		}

		ds->imageDescriptorsCount = imageDescriptorCount;
		ds->bufferDescriptorsCount = bufferDescriptorCount;
		ds->texelBufferDescriptorsCount = texelBufferDescriptorCount;

		ds->imageDescriptors = 0;
		ds->bufferDescriptors = 0;
		ds->texelBufferDescriptors = 0;

		if(imageDescriptorCount > 0)
		{
			ds->imageDescriptors = consecutivePoolAllocate(dp->imageDescriptorCPA, imageDescriptorCount);
		}

		if(bufferDescriptorCount)
		{
			ds->bufferDescriptors = consecutivePoolAllocate(dp->bufferDescriptorCPA, bufferDescriptorCount);
		}

		if(texelBufferDescriptorCount)
		{
			ds->texelBufferDescriptors = consecutivePoolAllocate(dp->texelBufferDescriptorCPA, texelBufferDescriptorCount);
		}

		//TODO immutable samplers

		//TODO maybe we could sort them in place
		//based on binding number
		uint32_t imageDescriptorCounter = 0, bufferDescriptorCounter = 0, texelBufferDescriptorCounter = 0;
		for(uint32_t d = 0; d < dsl->bindingsCount; ++d)
		{
			switch(dsl->bindings[d].descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				ds->imageDescriptors[imageDescriptorCounter].binding = dsl->bindings[d].binding;
				ds->imageDescriptors[imageDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->imageDescriptors[imageDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->imageDescriptors[imageDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
				imageDescriptorCounter += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				ds->imageDescriptors[bufferDescriptorCounter].binding = dsl->bindings[d].binding;
				ds->imageDescriptors[bufferDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->imageDescriptors[bufferDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->imageDescriptors[bufferDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
				bufferDescriptorCounter += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				ds->imageDescriptors[texelBufferDescriptorCounter].binding = dsl->bindings[d].binding;
				ds->imageDescriptors[texelBufferDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->imageDescriptors[texelBufferDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->imageDescriptors[texelBufferDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
				texelBufferDescriptorCounter += dsl->bindings[d].descriptorCount;
				break;
			}
		}
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorSetLayout*                      pSetLayout)
{
	assert(device);
	assert(pCreateInfo);

	_descriptorSetLayout* dsl = ALLOCATE(sizeof(_descriptorSetLayout), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dsl)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	dsl->bindings = ALLOCATE(sizeof(VkDescriptorSetLayoutBinding), pCreateInfo->bindingCount, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dsl->bindings)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(dsl->bindings, pCreateInfo->bindingCount, sizeof(VkDescriptorSetLayoutBinding)*pCreateInfo->bindingCount);

	dsl->flags = pCreateInfo->flags;

	*pSetLayout = dsl;

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(
	VkDevice                                    device,
	uint32_t                                    descriptorWriteCount,
	const VkWriteDescriptorSet*                 pDescriptorWrites,
	uint32_t                                    descriptorCopyCount,
	const VkCopyDescriptorSet*                  pDescriptorCopies)
{
	assert(device);

	for(uint32_t c = 0; c < descriptorWriteCount; ++c)
	{
		_descriptorSet* ds = pDescriptorWrites[c].dstSet;

		switch(pDescriptorWrites[c].descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		{
			_descriptorImage* di = ds->imageDescriptors;
			for(uint32_t d = 0; d < ds->imageDescriptorsCount; ++d)
			{
				if(ds->imageDescriptors[d].binding == pDescriptorWrites[c].dstBinding)
				{
					//found it
					di = ds->imageDescriptors + d;
					break;
				}
			}

			di += pDescriptorWrites[c].dstArrayElement;
			for(uint32_t d = 0; d < pDescriptorWrites[c].descriptorCount; ++d, di++)
			{
				di->imageLayout = pDescriptorWrites[c].pImageInfo[d].imageLayout;
				di->imageView = pDescriptorWrites[c].pImageInfo[d].imageView;
				di->sampler = pDescriptorWrites[c].pImageInfo[d].sampler;
			}
			break;
		}
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		{
			_descriptorBuffer* di = ds->bufferDescriptors;
			for(uint32_t d = 0; d < ds->bufferDescriptorsCount; ++d)
			{
				if(ds->bufferDescriptors[d].binding == pDescriptorWrites[c].dstBinding)
				{
					//found it
					di = ds->bufferDescriptors + d;
					break;
				}
			}

			di += pDescriptorWrites[c].dstArrayElement;
			for(uint32_t d = 0; d < pDescriptorWrites[c].descriptorCount; ++d, di++)
			{
				di->buffer = pDescriptorWrites[c].pBufferInfo[d].buffer;
				di->offset = pDescriptorWrites[c].pBufferInfo[d].offset;
				di->range = pDescriptorWrites[c].pBufferInfo[d].range;
			}
			break;
		}
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		{
			_descriptorTexelBuffer* di = ds->texelBufferDescriptors;
			for(uint32_t d = 0; d < ds->texelBufferDescriptorsCount; ++d)
			{
				if(ds->texelBufferDescriptors[d].binding == pDescriptorWrites[c].dstBinding)
				{
					//found it
					di = ds->texelBufferDescriptors + d;
					break;
				}
			}

			di += pDescriptorWrites[c].dstArrayElement;
			for(uint32_t d = 0; d < pDescriptorWrites[c].descriptorCount; ++d, di++)
			{
				di->bufferView = pDescriptorWrites[c].pTexelBufferView[d];
			}
			break;
		}
		}
	}

	for(uint32_t c = 0; c < descriptorCopyCount; ++c)
	{
		_descriptorSet* sds = pDescriptorCopies[c].srcSet;
		_descriptorSet* dds = pDescriptorCopies[c].dstSet;

		for(uint32_t d = 0; d < sds->imageDescriptorsCount; ++d)
		{
			if(sds->imageDescriptors[d].binding == pDescriptorCopies[c].srcBinding)
			{
				_descriptorImage* sdi = sds->imageDescriptors + d + pDescriptorCopies[c].srcArrayElement;

				for(uint32_t e = 0; e < dds->imageDescriptorsCount; ++e)
				{
					if(dds->imageDescriptors[e].binding == pDescriptorCopies[c].dstBinding)
					{
						_descriptorImage* ddi = dds->imageDescriptors + e + pDescriptorCopies[c].dstArrayElement;
						memcpy(ddi, sdi, sizeof(_descriptorImage) * pDescriptorCopies[c].descriptorCount);
						break;
					}
				}

				break;
			}
		}

		for(uint32_t d = 0; d < sds->bufferDescriptorsCount; ++d)
		{
			if(sds->bufferDescriptors[d].binding == pDescriptorCopies[c].srcBinding)
			{
				_descriptorBuffer* sdi = sds->bufferDescriptors + d + pDescriptorCopies[c].srcArrayElement;

				for(uint32_t e = 0; e < dds->bufferDescriptorsCount; ++e)
				{
					if(dds->bufferDescriptors[e].binding == pDescriptorCopies[c].dstBinding)
					{
						_descriptorBuffer* ddi = dds->bufferDescriptors + e + pDescriptorCopies[c].dstArrayElement;
						memcpy(ddi, sdi, sizeof(_descriptorBuffer) * pDescriptorCopies[c].descriptorCount);
						break;
					}
				}

				break;
			}
		}

		for(uint32_t d = 0; d < sds->texelBufferDescriptorsCount; ++d)
		{
			if(sds->texelBufferDescriptors[d].binding == pDescriptorCopies[c].srcBinding)
			{
				_descriptorTexelBuffer* sdi = sds->texelBufferDescriptors + d + pDescriptorCopies[c].srcArrayElement;

				for(uint32_t e = 0; e < dds->imageDescriptorsCount; ++e)
				{
					if(dds->texelBufferDescriptors[e].binding == pDescriptorCopies[c].dstBinding)
					{
						_descriptorTexelBuffer* ddi = dds->texelBufferDescriptors + e + pDescriptorCopies[c].dstArrayElement;
						memcpy(ddi, sdi, sizeof(_descriptorTexelBuffer) * pDescriptorCopies[c].descriptorCount);
						break;
					}
				}

				break;
			}
		}
	}
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	VkDescriptorPoolResetFlags                  flags)
{
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipelineLayout                            layout,
	uint32_t                                    firstSet,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets,
	uint32_t                                    dynamicOffsetCount,
	const uint32_t*                             pDynamicOffsets)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(
	VkDevice                                    device,
	VkDescriptorSetLayout                       descriptorSetLayout,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets)
{
	return VK_SUCCESS;
}


VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplate(
	VkDevice                                    device,
	const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate)
{

}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplate(
	VkDevice                                    device,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const VkAllocationCallbacks*                pAllocator)
{

}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplate(
	VkDevice                                    device,
	VkDescriptorSet                             descriptorSet,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const void*                                 pData)
{

}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupport(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	VkDescriptorSetLayoutSupport*               pSupport)
{

}
