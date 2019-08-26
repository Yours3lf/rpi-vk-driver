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

	void* dsmem = ALLOCATE(sizeof(_descriptorSet)*pCreateInfo->maxSets, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!dsmem)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	dp->descriptorSetPA = createPoolAllocator(dsmem, sizeof(_descriptorSet), sizeof(_descriptorSet) * pCreateInfo->maxSets);
	dp->imageDescriptorCPA = 0;
	dp->bufferDescriptorCPA = 0;
	dp->texelBufferDescriptorCPA = 0;

	void* memem = ALLOCATE(sizeof(mapElem)*imageDescriptorCount + bufferDescriptorCount + texelBufferDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!memem)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}
	dp->mapElementCPA = createConsecutivePoolAllocator(memem, sizeof(mapElem), sizeof(mapElem) * (imageDescriptorCount + bufferDescriptorCount + texelBufferDescriptorCount));

	if(imageDescriptorCount > 0)
	{
		dp->imageDescriptorCPA = ALLOCATE(sizeof(ConsecutivePoolAllocator), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!dp)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		void* mem = ALLOCATE(sizeof(_descriptorImage)*imageDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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

		void* mem = ALLOCATE(sizeof(_descriptorBuffer)*bufferDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
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

		void* mem = ALLOCATE(sizeof(_descriptorTexelBuffer)*texelBufferDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		*dp->texelBufferDescriptorCPA = createConsecutivePoolAllocator(mem, sizeof(_descriptorTexelBuffer), sizeof(_descriptorTexelBuffer) * texelBufferDescriptorCount);
	}

	*pDescriptorPool = dp;

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
			ds->imageBindingMap = createMap(consecutivePoolAllocate(&dp->mapElementCPA, imageDescriptorCount), imageDescriptorCount);
		}

		if(bufferDescriptorCount > 0)
		{
			ds->bufferDescriptors = consecutivePoolAllocate(dp->bufferDescriptorCPA, bufferDescriptorCount);
			ds->bufferBindingMap = createMap(consecutivePoolAllocate(&dp->mapElementCPA, bufferDescriptorCount), bufferDescriptorCount);
		}

		if(texelBufferDescriptorCount > 0)
		{
			ds->texelBufferDescriptors = consecutivePoolAllocate(dp->texelBufferDescriptorCPA, texelBufferDescriptorCount);
			ds->texelBufferBindingMap = createMap(consecutivePoolAllocate(&dp->mapElementCPA, texelBufferDescriptorCount), texelBufferDescriptorCount);
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
				setMapElement(&ds->imageBindingMap, dsl->bindings[d].binding, &ds->imageDescriptors[imageDescriptorCounter]);
				ds->imageDescriptors[imageDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->imageDescriptors[imageDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->imageDescriptors[imageDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
				imageDescriptorCounter += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				setMapElement(&ds->bufferBindingMap, dsl->bindings[d].binding, &ds->bufferDescriptors[bufferDescriptorCounter]);
				ds->bufferDescriptors[bufferDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->bufferDescriptors[bufferDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->bufferDescriptors[bufferDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
				bufferDescriptorCounter += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				setMapElement(&ds->texelBufferBindingMap, dsl->bindings[d].binding, &ds->texelBufferDescriptors[texelBufferDescriptorCounter]);
				ds->texelBufferDescriptors[texelBufferDescriptorCounter].count = dsl->bindings[d].descriptorCount;
				ds->texelBufferDescriptors[texelBufferDescriptorCounter].type = dsl->bindings[d].descriptorType;
				ds->texelBufferDescriptors[texelBufferDescriptorCounter].stageFlags = dsl->bindings[d].stageFlags;
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

	dsl->bindings = ALLOCATE(sizeof(VkDescriptorSetLayoutBinding)*pCreateInfo->bindingCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dsl->bindings)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(dsl->bindings, pCreateInfo->pBindings, sizeof(VkDescriptorSetLayoutBinding)*pCreateInfo->bindingCount);

	dsl->flags = pCreateInfo->flags;
	dsl->bindingsCount = pCreateInfo->bindingCount;

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
			_descriptorImage* di = getMapElement(ds->imageBindingMap, pDescriptorWrites[c].dstBinding);
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
			_descriptorBuffer* di = getMapElement(ds->bufferBindingMap, pDescriptorWrites[c].dstBinding);
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
			_descriptorTexelBuffer* di = getMapElement(ds->texelBufferBindingMap, pDescriptorWrites[c].dstBinding);
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

		_descriptorImage* sdi = getMapElement(sds->imageBindingMap, pDescriptorCopies[c].srcBinding);
		if(sdi)
		{
			_descriptorImage* ddi = getMapElement(dds->imageBindingMap, pDescriptorCopies[c].dstBinding);
			sdi += pDescriptorCopies[c].srcArrayElement;
			ddi += pDescriptorCopies[c].dstArrayElement;
			memcpy(ddi, sdi, sizeof(_descriptorImage) * pDescriptorCopies[c].descriptorCount);
		}

		_descriptorBuffer* sdb = getMapElement(sds->bufferBindingMap, pDescriptorCopies[c].srcBinding);
		if(sdb)
		{
			_descriptorBuffer* ddb = getMapElement(dds->bufferBindingMap, pDescriptorCopies[c].dstBinding);
			sdb += pDescriptorCopies[c].srcArrayElement;
			ddb += pDescriptorCopies[c].dstArrayElement;
			memcpy(ddb, sdb, sizeof(_descriptorBuffer) * pDescriptorCopies[c].descriptorCount);
		}

		_descriptorTexelBuffer* sdtb = getMapElement(sds->texelBufferBindingMap, pDescriptorCopies[c].srcBinding);
		if(sdtb)
		{
			_descriptorTexelBuffer* ddtb = getMapElement(dds->texelBufferBindingMap, pDescriptorCopies[c].dstBinding);
			sdtb += pDescriptorCopies[c].srcArrayElement;
			ddtb += pDescriptorCopies[c].dstArrayElement;
			memcpy(ddtb, sdtb, sizeof(_descriptorTexelBuffer) * pDescriptorCopies[c].descriptorCount);
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
	//TODO dynamic offsets

	assert(commandBuffer);
	assert(layout);
	assert(pDescriptorSets);

	_commandBuffer* cb = commandBuffer;

	//use pipeline layout's memory to store what is bound...

	_pipelineLayout* pl = pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? cb->graphicsPipeline->layout : cb->computePipeline->layout;
	assert(firstSet + descriptorSetCount <= pl->setLayoutCount);

	for(uint32_t c = firstSet; c < firstSet + descriptorSetCount; ++c)
	{
		setMapElement(&pl->descriptorSetBindingMap, c, pDescriptorSets[c]);
	}
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
