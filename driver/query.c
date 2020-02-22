#include "common.h"

//TODO VkPerformanceQuerySubmitInfoKHR

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireProfilingLockKHR(
	VkDevice                                    device,
	const VkAcquireProfilingLockInfoKHR*        pInfo)
{
	//TODO
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkReleaseProfilingLockKHR(
	VkDevice                                    device)
{
	//TODO
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateQueryPool(
	VkDevice                                    device,
	const VkQueryPoolCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkQueryPool*                                pQueryPool)
{
	assert(device);
	assert(pQueryPool);

	_queryPool* qp = ALLOCATE(sizeof(_queryPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION ||
	   pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS ||
	   pCreateInfo->queryType == VK_QUERY_TYPE_TIMESTAMP)
	{
		UNSUPPORTED(VK_QUERY_TYPE_OCCLUSION);
	}
	else if(pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR)
	{
		qp->queryCount = pCreateInfo->queryCount;
		qp->type = pCreateInfo->queryType;
		qp->queryPool = ALLOCATE(sizeof(_query) * qp->queryCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

		VkQueryPoolPerformanceCreateInfoKHR ci = *(VkQueryPoolPerformanceCreateInfoKHR*)pCreateInfo->pNext;

		for(uint32_t c = 0; c < qp->queryCount; ++c)
		{
			assert(ci.counterIndexCount <= VC4_PERFCNT_NUM_EVENTS);
			qp->queryPool[c].numEnabledCounters = ci.counterIndexCount;
			memcpy(qp->queryPool[c].enabledCounters, ci.pCounterIndices, sizeof(uint32_t) * ci.counterIndexCount);

			for(uint32_t d = 0; d < ci.counterIndexCount; d += DRM_VC4_MAX_PERF_COUNTERS)
			{
				qp->queryPool[c].perfmonIDs[d] = vc4_create_perfmon(controlFd, &qp->queryPool[c].enabledCounters[d], qp->queryPool[c].numEnabledCounters > DRM_VC4_MAX_PERF_COUNTERS ? DRM_VC4_MAX_PERF_COUNTERS : qp->queryPool[c].numEnabledCounters);
			}
		}

		*pQueryPool = qp;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdResetQueryPool(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount)
{
	//TODO
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyQueryPool(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(queryPool);

	_queryPool* qp = queryPool;

	for(uint32_t c = 0; c < qp->queryCount; ++c)
	{
		for(uint32_t d = 0; d < qp->queryPool[c].enabledCounters; d += DRM_VC4_MAX_PERF_COUNTERS)
		{
			vc4_destroy_perfmon(controlFd, qp->queryPool[c].perfmonIDs[d]);
		}
	}

	FREE(qp->queryPool);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdEndQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	assert(commandBuffer);
	assert(queryPool);

	//TODO
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBeginQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query,
	VkQueryControlFlags                         flags)
{
	assert(commandBuffer);
	assert(queryPool);

	//TODO
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyQueryPoolResults(
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

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetQueryPoolResults(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	size_t                                      dataSize,
	void*                                       pData,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags)
{
	assert(device);
	assert(queryPool);

	//TODO flags

	_queryPool* qp = queryPool;

	for(uint32_t c = firstQuery; c < queryCount; ++c)
	{
		uint32_t counter = 0;
		for(uint32_t d = 0; d < dataSize; d += stride, ++counter)
		{
			VkPerformanceCounterResultKHR* result = ((char*)pData) + d;
			result->uint64 = qp->queryPool[c].counterValues[counter];
		}
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdWriteTimestamp(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlagBits                     pipelineStage,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	//TODO
}
