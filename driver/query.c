#include "common.h"

//TODO VkPerformanceQuerySubmitInfoKHR
//TODO query test

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAcquireProfilingLockKHR(
	VkDevice                                    device,
	const VkAcquireProfilingLockInfoKHR*        pInfo)
{
	PROFILESTART(rpi_vkAcquireProfilingLockKHR);
	//TODO
	PROFILEEND(rpi_vkAcquireProfilingLockKHR);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkReleaseProfilingLockKHR(
	VkDevice                                    device)
{
	PROFILESTART(rpi_vkReleaseProfilingLockKHR);
	//TODO
	PROFILEEND(rpi_vkReleaseProfilingLockKHR);
}

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateQueryPool(
	VkDevice                                    device,
	const VkQueryPoolCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkQueryPool*                                pQueryPool)
{
	PROFILESTART(rpi_vkCreateQueryPool);

	assert(device);
	assert(pQueryPool);

	_queryPool* qp = ALLOCATE(sizeof(_queryPool), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION)
	{
		UNSUPPORTED(VK_QUERY_TYPE_OCCLUSION);
	}
	else if(pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
	{
		UNSUPPORTED(VK_QUERY_TYPE_PIPELINE_STATISTICS);
	}
	else if(pCreateInfo->queryType == VK_QUERY_TYPE_TIMESTAMP)
	{
		UNSUPPORTED(VK_QUERY_TYPE_TIMESTAMP);
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
				qp->queryPool[c].perfmonIDs[d / DRM_VC4_MAX_PERF_COUNTERS] = vc4_create_perfmon(controlFd, &qp->queryPool[c].enabledCounters[d], qp->queryPool[c].numEnabledCounters > DRM_VC4_MAX_PERF_COUNTERS ? DRM_VC4_MAX_PERF_COUNTERS : qp->queryPool[c].numEnabledCounters);
				memset(&qp->queryPool[c].counterValues[d / DRM_VC4_MAX_PERF_COUNTERS][0], 0, sizeof(uint64_t) * DRM_VC4_MAX_PERF_COUNTERS);
			}
		}

		*pQueryPool = qp;
	}

	PROFILEEND(rpi_vkCreateQueryPool);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdResetQueryPool(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount)
{
	PROFILESTART(rpi_vkCmdResetQueryPool);
	//TODO
	PROFILEEND(rpi_vkCmdResetQueryPool);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyQueryPool(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	const VkAllocationCallbacks*                pAllocator)
{
	PROFILESTART(rpi_vkDestroyQueryPool);

	assert(device);
	assert(queryPool);

	_queryPool* qp = queryPool;

	for(uint32_t c = 0; c < qp->queryCount; ++c)
	{
		for(uint32_t d = 0; d < qp->queryPool[c].numEnabledCounters; d += DRM_VC4_MAX_PERF_COUNTERS)
		{
			vc4_destroy_perfmon(controlFd, qp->queryPool[c].perfmonIDs[d / DRM_VC4_MAX_PERF_COUNTERS]);
		}
	}

	FREE(qp->queryPool);

	PROFILEEND(rpi_vkDestroyQueryPool);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdEndQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	PROFILESTART(rpi_vkCmdEndQuery);

	assert(commandBuffer);
	assert(queryPool);

	_commandBuffer* cmdBuf = commandBuffer;

	cmdBuf->perfmonID = 0;

	PROFILEEND(rpi_vkCmdEndQuery);
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBeginQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query,
	VkQueryControlFlags                         flags)
{
	PROFILESTART(rpi_vkCmdBeginQuery);

	assert(commandBuffer);
	assert(queryPool);

	//TODO flags

	_commandBuffer* cmdBuf = commandBuffer;
	_queryPool* qp = queryPool;


	//pass id will select the perfmon at submit
	cmdBuf->perfmonID = qp->queryPool[query].perfmonIDs;

	PROFILEEND(rpi_vkCmdBeginQuery);
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
	PROFILESTART(rpi_vkCmdCopyQueryPoolResults);

	//TODO

	PROFILEEND(rpi_vkCmdCopyQueryPoolResults);
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
	PROFILESTART(rpi_vkGetQueryPoolResults);

	assert(device);
	assert(queryPool);

	//TODO flags
	//TODO return values etc.

	_queryPool* qp = queryPool;

	for(uint32_t c = firstQuery; c < queryCount; ++c)
	{
		for(uint32_t d = 0; d < qp->queryPool[c].numEnabledCounters; d += DRM_VC4_MAX_PERF_COUNTERS)
		{
			vc4_perfmon_get_values(controlFd, qp->queryPool[c].perfmonIDs[d / DRM_VC4_MAX_PERF_COUNTERS], &qp->queryPool[c].counterValues[d / DRM_VC4_MAX_PERF_COUNTERS][0]);
		}

		uint32_t counter = 0;
		for(uint32_t d = 0; d < dataSize; d += stride, ++counter)
		{
			VkPerformanceCounterResultKHR* result = ((char*)pData) + d;
			result->uint64 = qp->queryPool[c].counterValues[counter / DRM_VC4_MAX_PERF_COUNTERS][counter % DRM_VC4_MAX_PERF_COUNTERS];
		}
	}

	PROFILEEND(rpi_vkGetQueryPoolResults);
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdWriteTimestamp(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlagBits                     pipelineStage,
	VkQueryPool                                 queryPool,
	uint32_t                                    query)
{
	UNSUPPORTED(VK_QUERY_TYPE_TIMESTAMP);
}
