#include "common.h"

#include "declarations.h"

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorPool)(
	VkDevice                                    device,
	const VkDescriptorPoolCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorPool*                           pDescriptorPool)
{
	PROFILESTART(RPIFUNC(vkCreateDescriptorPool));

	assert(device);
	assert(pCreateInfo);

	_descriptorPool* dp = ALLOCATE(sizeof(_descriptorPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dp)
	{
		PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

#ifdef DEBUG_BUILD
	memset(dp, 0, sizeof(_descriptorPool));
#endif

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
			imageDescriptorCount += pCreateInfo->pPoolSizes[c].descriptorCount;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			bufferDescriptorCount += pCreateInfo->pPoolSizes[c].descriptorCount;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			texelBufferDescriptorCount += pCreateInfo->pPoolSizes[c].descriptorCount;
			break;
		default:
			assert(0);
			break;
		}
	}

	dp->freeAble = pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	void* dsmem = ALLOCATE(sizeof(_descriptorSet)*pCreateInfo->maxSets, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!dsmem)
	{
		PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	dp->descriptorSetPA = createPoolAllocator(dsmem, sizeof(_descriptorSet), sizeof(_descriptorSet) * pCreateInfo->maxSets);


	uint32_t mapElemBlockSize = sizeof(mapElem);
	uint32_t mapBufSize = mapElemBlockSize * (imageDescriptorCount + bufferDescriptorCount + texelBufferDescriptorCount);
	void* memem = ALLOCATE(mapBufSize, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	if(!memem)
	{
		PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}
	dp->mapElementCPA = createConsecutivePoolAllocator(memem, mapElemBlockSize, mapBufSize);

	if(imageDescriptorCount > 0)
	{
		uint32_t blockSize = sizeof(_descriptorImage);
		void* mem = ALLOCATE(blockSize*imageDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		dp->imageDescriptorCPA = createConsecutivePoolAllocator(mem, blockSize, blockSize * imageDescriptorCount);
	}

	if(bufferDescriptorCount > 0)
	{
		uint32_t blockSize = sizeof(_descriptorBuffer);
		void* mem = ALLOCATE(blockSize*bufferDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		dp->bufferDescriptorCPA = createConsecutivePoolAllocator(mem, blockSize, blockSize * bufferDescriptorCount);
	}

	if(texelBufferDescriptorCount > 0)
	{
		uint32_t blockSize = sizeof(_descriptorBuffer);
		void* mem = ALLOCATE(blockSize*texelBufferDescriptorCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
		if(!mem)
		{
			PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		dp->texelBufferDescriptorCPA = createConsecutivePoolAllocator(mem, blockSize, blockSize * texelBufferDescriptorCount);
	}

	*pDescriptorPool = dp;

	PROFILEEND(RPIFUNC(vkCreateDescriptorPool));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAllocateDescriptorSets)(
	VkDevice                                    device,
	const VkDescriptorSetAllocateInfo*          pAllocateInfo,
	VkDescriptorSet*                            pDescriptorSets)
{
	PROFILESTART(RPIFUNC(vkAllocateDescriptorSets));

	assert(device);

	_descriptorPool* dp = pAllocateInfo->descriptorPool;

	for(uint32_t c = 0; c < pAllocateInfo->descriptorSetCount; ++c)
	{
		_descriptorSet* ds = poolAllocate(&dp->descriptorSetPA);
		pDescriptorSets[c] = ds;

		_descriptorSetLayout* dsl = pAllocateInfo->pSetLayouts[c];

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
				imageDescriptorCount += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				bufferDescriptorCount += dsl->bindings[d].descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				texelBufferDescriptorCount += dsl->bindings[d].descriptorCount;
				break;
			default:
				assert(0);
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
			ds->imageDescriptors = getCPAptrFromOffset(&dp->imageDescriptorCPA, consecutivePoolAllocate(&dp->imageDescriptorCPA, imageDescriptorCount));
			ds->imageBindingMap = createMap(getCPAptrFromOffset(&dp->mapElementCPA, consecutivePoolAllocate(&dp->mapElementCPA, imageDescriptorCount)), imageDescriptorCount);
		}

		if(bufferDescriptorCount > 0)
		{
			ds->bufferDescriptors = getCPAptrFromOffset(&dp->bufferDescriptorCPA, consecutivePoolAllocate(&dp->bufferDescriptorCPA, bufferDescriptorCount));
			ds->bufferBindingMap = createMap(getCPAptrFromOffset(&dp->mapElementCPA, consecutivePoolAllocate(&dp->mapElementCPA, bufferDescriptorCount)), bufferDescriptorCount);
		}

		if(texelBufferDescriptorCount > 0)
		{
			ds->texelBufferDescriptors = getCPAptrFromOffset(&dp->texelBufferDescriptorCPA, consecutivePoolAllocate(&dp->texelBufferDescriptorCPA, texelBufferDescriptorCount));
			ds->texelBufferBindingMap = createMap(getCPAptrFromOffset(&dp->mapElementCPA, consecutivePoolAllocate(&dp->mapElementCPA, texelBufferDescriptorCount)), texelBufferDescriptorCount);
		}

		//TODO immutable samplers

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
			default:
				assert(0);
				break;
			}
		}
	}

	PROFILEEND(RPIFUNC(vkAllocateDescriptorSets));
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorSetLayout)(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorSetLayout*                      pSetLayout)
{
	PROFILESTART(RPIFUNC(vkCreateDescriptorSetLayout));

	assert(device);
	assert(pCreateInfo);

	_descriptorSetLayout* dsl = ALLOCATE(sizeof(_descriptorSetLayout), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dsl)
	{
		PROFILEEND(RPIFUNC(vkCreateDescriptorSetLayout));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	dsl->bindings = ALLOCATE(sizeof(VkDescriptorSetLayoutBinding)*pCreateInfo->bindingCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!dsl->bindings)
	{
		FREE(dsl);
		PROFILEEND(RPIFUNC(vkCreateDescriptorSetLayout));
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	memcpy(dsl->bindings, pCreateInfo->pBindings, sizeof(VkDescriptorSetLayoutBinding)*pCreateInfo->bindingCount);

	//TODO immutable samplers

	dsl->flags = pCreateInfo->flags;
	dsl->bindingsCount = pCreateInfo->bindingCount;

	*pSetLayout = dsl;

	PROFILEEND(RPIFUNC(vkCreateDescriptorSetLayout));
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkUpdateDescriptorSets)(
	VkDevice                                    device,
	uint32_t                                    descriptorWriteCount,
	const VkWriteDescriptorSet*                 pDescriptorWrites,
	uint32_t                                    descriptorCopyCount,
	const VkCopyDescriptorSet*                  pDescriptorCopies)
{
	PROFILESTART(RPIFUNC(vkUpdateDescriptorSets));

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
		default:
		{
			assert(0);
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

	PROFILEEND(RPIFUNC(vkUpdateDescriptorSets));
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetDescriptorPool)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	VkDescriptorPoolResetFlags                  flags)
{
	PROFILESTART(RPIFUNC(vkResetDescriptorPool));
	//TODO
	PROFILEEND(RPIFUNC(vkResetDescriptorPool));
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorPool)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyDescriptorPool));

	assert(device);
	assert(descriptorPool);

	_descriptorPool* dp = descriptorPool;

	FREE(dp->descriptorSetPA.buf);
	FREE(dp->mapElementCPA.buf);
	FREE(dp->imageDescriptorCPA.buf);
	FREE(dp->texelBufferDescriptorCPA.buf);
	FREE(dp->bufferDescriptorCPA.buf);

	FREE(dp);

	PROFILEEND(RPIFUNC(vkDestroyDescriptorPool));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindDescriptorSets)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipelineLayout                            layout,
	uint32_t                                    firstSet,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets,
	uint32_t                                    dynamicOffsetCount,
	const uint32_t*                             pDynamicOffsets)
{
	PROFILESTART(RPIFUNC(vkCmdBindDescriptorSets));

	//TODO dynamic offsets

	assert(commandBuffer);
	assert(layout);
	assert(pDescriptorSets);

	_commandBuffer* cb = commandBuffer;

	//use pipeline layout's memory to store what is bound...

	_pipelineLayout* pl = layout;//pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? cb->graphicsPipeline->layout : cb->computePipeline->layout;
	assert(firstSet + descriptorSetCount <= pl->setLayoutCount);

	for(uint32_t c = 0; c < descriptorSetCount; ++c)
	{
		setMapElement(&pl->descriptorSetBindingMap, firstSet + c, pDescriptorSets[c]);
	}

	cb->descriptorSetDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdBindDescriptorSets));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorSetLayout)(
	VkDevice                                    device,
	VkDescriptorSetLayout                       descriptorSetLayout,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyDescriptorSetLayout));

	assert(device);
	assert(descriptorSetLayout);

	_descriptorSetLayout* dsl = descriptorSetLayout;

	FREE(dsl->bindings);

	FREE(dsl);

	PROFILEEND(RPIFUNC(vkDestroyDescriptorSetLayout));
}

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkFreeDescriptorSets)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets)
{
	PROFILESTART(RPIFUNC(vkFreeDescriptorSets));

	assert(device);
	assert(descriptorPool);

	_descriptorPool* dp = descriptorPool;
	assert(dp->freeAble);

	for(uint32_t c = 0; c < descriptorSetCount; ++c)
	{
		_descriptorSet* ds = pDescriptorSets[c];

		if(ds->imageDescriptorsCount > 0)
		{
			consecutivePoolFree(&dp->mapElementCPA, ds->imageBindingMap.elements, ds->imageDescriptorsCount);
			consecutivePoolFree(&dp->imageDescriptorCPA, ds->imageDescriptors, ds->imageDescriptorsCount);
		}

		if(ds->bufferDescriptorsCount > 0)
		{
			consecutivePoolFree(&dp->mapElementCPA, ds->bufferBindingMap.elements, ds->bufferDescriptorsCount);
			consecutivePoolFree(&dp->bufferDescriptorCPA, ds->bufferDescriptors, ds->bufferDescriptorsCount);
		}

		if(ds->texelBufferDescriptorsCount > 0)
		{
			consecutivePoolFree(&dp->mapElementCPA, ds->texelBufferBindingMap.elements, ds->texelBufferDescriptorsCount);
			consecutivePoolFree(&dp->texelBufferDescriptorCPA, ds->texelBufferDescriptors, ds->texelBufferDescriptorsCount);
		}

		poolFree(&dp->descriptorSetPA, ds);
	}

	PROFILEEND(RPIFUNC(vkFreeDescriptorSets));
	return VK_SUCCESS;
}


VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorUpdateTemplate)(
	VkDevice                                    device,
	const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate)
{
	PROFILESTART(RPIFUNC(vkCreateDescriptorUpdateTemplate));
	//TODO
	PROFILEEND(RPIFUNC(vkCreateDescriptorUpdateTemplate));

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorUpdateTemplate)(
	VkDevice                                    device,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(RPIFUNC(vkDestroyDescriptorUpdateTemplate));
	//TODO
	PROFILEEND(RPIFUNC(vkDestroyDescriptorUpdateTemplate));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkUpdateDescriptorSetWithTemplate)(
	VkDevice                                    device,
	VkDescriptorSet                             descriptorSet,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const void*                                 pData)
{
	PROFILESTART(RPIFUNC(vkUpdateDescriptorSetWithTemplate));
	//TODO
	PROFILEEND(RPIFUNC(vkUpdateDescriptorSetWithTemplate));
}

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDescriptorSetLayoutSupport)(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	VkDescriptorSetLayoutSupport*               pSupport)
{
	PROFILESTART(RPIFUNC(vkGetDescriptorSetLayoutSupport));
	//TODO
	PROFILEEND(RPIFUNC(vkGetDescriptorSetLayoutSupport));
}
