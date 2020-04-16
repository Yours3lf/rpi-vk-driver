#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateInstance(
	const VkInstanceCreateInfo*                 pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkInstance*                                 pInstance);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyInstance(
	VkInstance                                  instance,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumeratePhysicalDevices(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceCount,
	VkPhysicalDevice*                           pPhysicalDevices);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFeatures(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures*                   pFeatures);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties*                         pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkImageTiling                               tiling,
	VkImageUsageFlags                           usage,
	VkImageCreateFlags                          flags,
	VkImageFormatProperties*                    pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceProperties(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties*                 pProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceQueueFamilyProperties(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pQueueFamilyPropertyCount,
	VkQueueFamilyProperties*                    pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceMemoryProperties(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties*           pMemoryProperties);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL rpi_vkGetInstanceProcAddr(
	VkInstance                                  instance,
	const char*                                 pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL rpi_vkGetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDevice(
	VkPhysicalDevice                            physicalDevice,
	const VkDeviceCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDevice*                                   pDevice);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyDevice(
	VkDevice                                    device,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateInstanceExtensionProperties(
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateDeviceExtensionProperties(
	VkPhysicalDevice                            physicalDevice,
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateInstanceLayerProperties(
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateDeviceLayerProperties(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceQueue(
	VkDevice                                    device,
	uint32_t                                    queueFamilyIndex,
	uint32_t                                    queueIndex,
	VkQueue*                                    pQueue);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkQueueSubmit(
	VkQueue                                     queue,
	uint32_t                                    submitCount,
	const VkSubmitInfo*                         pSubmits,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkQueueWaitIdle(
	VkQueue                                     queue);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkDeviceWaitIdle(
	VkDevice                                    device);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAllocateMemory(
	VkDevice                                    device,
	const VkMemoryAllocateInfo*                 pAllocateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDeviceMemory*                             pMemory);

VKAPI_ATTR void VKAPI_CALL rpi_vkFreeMemory(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkMapMemory(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize                                offset,
	VkDeviceSize                                size,
	VkMemoryMapFlags                            flags,
	void**                                      ppData);

VKAPI_ATTR void VKAPI_CALL rpi_vkUnmapMemory(
	VkDevice                                    device,
	VkDeviceMemory                              memory);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkFlushMappedMemoryRanges(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkInvalidateMappedMemoryRanges(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceMemoryCommitment(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize*                               pCommittedMemoryInBytes);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindBufferMemory(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindImageMemory(
	VkDevice                                    device,
	VkImage                                     image,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetBufferMemoryRequirements(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	VkMemoryRequirements*                       pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSparseMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements*            pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceSparseImageFormatProperties(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkSampleCountFlagBits                       samples,
	VkImageUsageFlags                           usage,
	VkImageTiling                               tiling,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties*              pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkQueueBindSparse(
	VkQueue                                     queue,
	uint32_t                                    bindInfoCount,
	const VkBindSparseInfo*                     pBindInfo,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateFence(
	VkDevice                                    device,
	const VkFenceCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFence*                                    pFence);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyFence(
	VkDevice                                    device,
	VkFence                                     fence,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkResetFences(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetFenceStatus(
	VkDevice                                    device,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkWaitForFences(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences,
	VkBool32                                    waitAll,
	uint64_t                                    timeout);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSemaphore(
	VkDevice                                    device,
	const VkSemaphoreCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSemaphore*                                pSemaphore);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySemaphore(
	VkDevice                                    device,
	VkSemaphore                                 semaphore,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateEvent(
	VkDevice                                    device,
	const VkEventCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkEvent*                                    pEvent);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyEvent(
	VkDevice                                    device,
	VkEvent                                     event,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetEventStatus(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkSetEvent(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkResetEvent(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateQueryPool(
	VkDevice                                    device,
	const VkQueryPoolCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkQueryPool*                                pQueryPool);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyQueryPool(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetQueryPoolResults(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	size_t                                      dataSize,
	void*                                       pData,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateBuffer(
	VkDevice                                    device,
	const VkBufferCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBuffer*                                   pBuffer);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyBuffer(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateBufferView(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyBufferView(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateImage(
	VkDevice                                    device,
	const VkImageCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImage*                                    pImage);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyImage(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSubresourceLayout(
	VkDevice                                    device,
	VkImage                                     image,
	const VkImageSubresource*                   pSubresource,
	VkSubresourceLayout*                        pLayout);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateImageView(
	VkDevice                                    device,
	const VkImageViewCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImageView*                                pView);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyImageView(
	VkDevice                                    device,
	VkImageView                                 imageView,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateShaderModule(
	VkDevice                                    device,
	const VkShaderModuleCreateInfo*             pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkShaderModule*                             pShaderModule);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyShaderModule(
	VkDevice                                    device,
	VkShaderModule                              shaderModule,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreatePipelineCache(
	VkDevice                                    device,
	const VkPipelineCacheCreateInfo*            pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineCache*                            pPipelineCache);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyPipelineCache(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPipelineCacheData(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	size_t*                                     pDataSize,
	void*                                       pData);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkMergePipelineCaches(
	VkDevice                                    device,
	VkPipelineCache                             dstCache,
	uint32_t                                    srcCacheCount,
	const VkPipelineCache*                      pSrcCaches);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateGraphicsPipelines(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkGraphicsPipelineCreateInfo*         pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateComputePipelines(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkComputePipelineCreateInfo*          pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyPipeline(
	VkDevice                                    device,
	VkPipeline                                  pipeline,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreatePipelineLayout(
	VkDevice                                    device,
	const VkPipelineLayoutCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineLayout*                           pPipelineLayout);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyPipelineLayout(
	VkDevice                                    device,
	VkPipelineLayout                            pipelineLayout,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSampler(
	VkDevice                                    device,
	const VkSamplerCreateInfo*                  pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSampler*                                  pSampler);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySampler(
	VkDevice                                    device,
	VkSampler                                   sampler,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDescriptorSetLayout(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorSetLayout*                      pSetLayout);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyDescriptorSetLayout(
	VkDevice                                    device,
	VkDescriptorSetLayout                       descriptorSetLayout,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDescriptorPool(
	VkDevice                                    device,
	const VkDescriptorPoolCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorPool*                           pDescriptorPool);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyDescriptorPool(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkResetDescriptorPool(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	VkDescriptorPoolResetFlags                  flags);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAllocateDescriptorSets(
	VkDevice                                    device,
	const VkDescriptorSetAllocateInfo*          pAllocateInfo,
	VkDescriptorSet*                            pDescriptorSets);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkFreeDescriptorSets(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets);

VKAPI_ATTR void VKAPI_CALL rpi_vkUpdateDescriptorSets(
	VkDevice                                    device,
	uint32_t                                    descriptorWriteCount,
	const VkWriteDescriptorSet*                 pDescriptorWrites,
	uint32_t                                    descriptorCopyCount,
	const VkCopyDescriptorSet*                  pDescriptorCopies);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateFramebuffer(
	VkDevice                                    device,
	const VkFramebufferCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFramebuffer*                              pFramebuffer);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyFramebuffer(
	VkDevice                                    device,
	VkFramebuffer                               framebuffer,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateRenderPass(
	VkDevice                                    device,
	const VkRenderPassCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkRenderPass*                               pRenderPass);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyRenderPass(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetRenderAreaGranularity(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	VkExtent2D*                                 pGranularity);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateCommandPool(
	VkDevice                                    device,
	const VkCommandPoolCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkCommandPool*                              pCommandPool);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkResetCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolResetFlags                     flags);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAllocateCommandBuffers(
	VkDevice                                    device,
	const VkCommandBufferAllocateInfo*          pAllocateInfo,
	VkCommandBuffer*                            pCommandBuffers);

VKAPI_ATTR void VKAPI_CALL rpi_vkFreeCommandBuffers(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBeginCommandBuffer(
	VkCommandBuffer                             commandBuffer,
	const VkCommandBufferBeginInfo*             pBeginInfo);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEndCommandBuffer(
	VkCommandBuffer                             commandBuffer);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkResetCommandBuffer(
	VkCommandBuffer                             commandBuffer,
	VkCommandBufferResetFlags                   flags);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBindPipeline(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipeline                                  pipeline);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetViewport(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstViewport,
	uint32_t                                    viewportCount,
	const VkViewport*                           pViewports);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetScissor(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstScissor,
	uint32_t                                    scissorCount,
	const VkRect2D*                             pScissors);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetLineWidth(
	VkCommandBuffer                             commandBuffer,
	float                                       lineWidth);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetDepthBias(
	VkCommandBuffer                             commandBuffer,
	float                                       depthBiasConstantFactor,
	float                                       depthBiasClamp,
	float                                       depthBiasSlopeFactor);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetBlendConstants(
	VkCommandBuffer                             commandBuffer,
	const float                                 blendConstants[4]);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetDepthBounds(
	VkCommandBuffer                             commandBuffer,
	float                                       minDepthBounds,
	float                                       maxDepthBounds);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetStencilCompareMask(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    compareMask);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetStencilWriteMask(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    writeMask);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetStencilReference(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    reference);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBindDescriptorSets(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipelineLayout                            layout,
	uint32_t                                    firstSet,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets,
	uint32_t                                    dynamicOffsetCount,
	const uint32_t*                             pDynamicOffsets);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBindIndexBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	VkIndexType                                 indexType);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBindVertexBuffers(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstBinding,
	uint32_t                                    bindingCount,
	const VkBuffer*                             pBuffers,
	const VkDeviceSize*                         pOffsets);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDraw(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    vertexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstVertex,
	uint32_t                                    firstInstance);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndexed(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    indexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstIndex,
	int32_t                                     vertexOffset,
	uint32_t                                    firstInstance);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDrawIndexedIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatch(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatchIndirect(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferCopy*                         pRegions);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageCopy*                          pRegions);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBlitImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageBlit*                          pRegions,
	VkFilter                                    filter);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyBufferToImage(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyImageToBuffer(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdUpdateBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                dataSize,
	const void*                                 pData);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdFillBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                size,
	uint32_t                                    data);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdClearColorImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearColorValue*                    pColor,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdClearDepthStencilImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearDepthStencilValue*             pDepthStencil,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdClearAttachments(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    attachmentCount,
	const VkClearAttachment*                    pAttachments,
	uint32_t                                    rectCount,
	const VkClearRect*                          pRects);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdResolveImage(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageResolve*                       pRegions);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetEvent(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdResetEvent(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdWaitEvents(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    eventCount,
	const VkEvent*                              pEvents,
	VkPipelineStageFlags                        srcStageMask,
	VkPipelineStageFlags                        dstStageMask,
	uint32_t                                    memoryBarrierCount,
	const VkMemoryBarrier*                      pMemoryBarriers,
	uint32_t                                    bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
	uint32_t                                    imageMemoryBarrierCount,
	const VkImageMemoryBarrier*                 pImageMemoryBarriers);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdPipelineBarrier(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlags                        srcStageMask,
	VkPipelineStageFlags                        dstStageMask,
	VkDependencyFlags                           dependencyFlags,
	uint32_t                                    memoryBarrierCount,
	const VkMemoryBarrier*                      pMemoryBarriers,
	uint32_t                                    bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
	uint32_t                                    imageMemoryBarrierCount,
	const VkImageMemoryBarrier*                 pImageMemoryBarriers);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBeginQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query,
	VkQueryControlFlags                         flags);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdEndQuery(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdResetQueryPool(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdWriteTimestamp(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlagBits                     pipelineStage,
	VkQueryPool                                 queryPool,
	uint32_t                                    query);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdCopyQueryPoolResults(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdPushConstants(
	VkCommandBuffer                             commandBuffer,
	VkPipelineLayout                            layout,
	VkShaderStageFlags                          stageFlags,
	uint32_t                                    offset,
	uint32_t                                    size,
	const void*                                 pValues);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdBeginRenderPass(
	VkCommandBuffer                             commandBuffer,
	const VkRenderPassBeginInfo*                pRenderPassBegin,
	VkSubpassContents                           contents);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdNextSubpass(
	VkCommandBuffer                             commandBuffer,
	VkSubpassContents                           contents);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdEndRenderPass(
	VkCommandBuffer                             commandBuffer);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdExecuteCommands(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumerateInstanceVersion(
	uint32_t*                                   pApiVersion);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindBufferMemory2(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindBufferMemoryInfo*               pBindInfos);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkBindImageMemory2(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindImageMemoryInfo*                pBindInfos);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceGroupPeerMemoryFeatures(
	VkDevice                                    device,
	uint32_t                                    heapIndex,
	uint32_t                                    localDeviceIndex,
	uint32_t                                    remoteDeviceIndex,
	VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdSetDeviceMask(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    deviceMask);

VKAPI_ATTR void VKAPI_CALL rpi_vkCmdDispatchBase(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    baseGroupX,
	uint32_t                                    baseGroupY,
	uint32_t                                    baseGroupZ,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumeratePhysicalDeviceGroups(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceGroupCount,
	VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageMemoryRequirements2(
	VkDevice                                    device,
	const VkImageMemoryRequirementsInfo2*       pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetBufferMemoryRequirements2(
	VkDevice                                    device,
	const VkBufferMemoryRequirementsInfo2*      pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetImageSparseMemoryRequirements2(
	VkDevice                                    device,
	const VkImageSparseMemoryRequirementsInfo2* pInfo,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFeatures2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures2*                  pFeatures);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties2*                pProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties2*                        pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceImageFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
	VkImageFormatProperties2*                   pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceQueueFamilyProperties2(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pQueueFamilyPropertyCount,
	VkQueueFamilyProperties2*                   pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceMemoryProperties2(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties2*          pMemoryProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceSparseImageFormatProperties2(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties2*             pProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkTrimCommandPool(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolTrimFlags                      flags);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDeviceQueue2(
	VkDevice                                    device,
	const VkDeviceQueueInfo2*                   pQueueInfo,
	VkQueue*                                    pQueue);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSamplerYcbcrConversion(
	VkDevice                                    device,
	const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSamplerYcbcrConversion*                   pYcbcrConversion);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySamplerYcbcrConversion(
	VkDevice                                    device,
	VkSamplerYcbcrConversion                    ycbcrConversion,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDescriptorUpdateTemplate(
	VkDevice                                    device,
	const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroyDescriptorUpdateTemplate(
	VkDevice                                    device,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL rpi_vkUpdateDescriptorSetWithTemplate(
	VkDevice                                    device,
	VkDescriptorSet                             descriptorSet,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const void*                                 pData);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalBufferProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
	VkExternalBufferProperties*                 pExternalBufferProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalFenceProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
	VkExternalFenceProperties*                  pExternalFenceProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceExternalSemaphoreProperties(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
	VkExternalSemaphoreProperties*              pExternalSemaphoreProperties);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetDescriptorSetLayoutSupport(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	VkDescriptorSetLayoutSupport*               pSupport);

VKAPI_ATTR void VKAPI_CALL rpi_vkDestroySurfaceKHR(
		VkInstance                                  instance,
		VkSurfaceKHR                                surface,
		const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateSwapchainKHR(
		VkDevice                                    device,
		const VkSwapchainCreateInfoKHR*             pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSwapchainKHR*                             pSwapchain);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceSurfaceSupportKHR(
		VkPhysicalDevice                            physicalDevice,
		uint32_t                                    queueFamilyIndex,
		VkSurfaceKHR                                surface,
		VkBool32*                                   pSupported);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceSurfaceFormatsKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pSurfaceFormatCount,
		VkSurfaceFormatKHR*                         pSurfaceFormats);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceSurfacePresentModesKHR(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pPresentModeCount,
		VkPresentModeKHR*                           pPresentModes);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetSwapchainImagesKHR(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint32_t*                                   pSwapchainImageCount,
		VkImage*                                    pSwapchainImages);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAcquireNextImageKHR(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint64_t                                    timeout,
		VkSemaphore                                 semaphore,
		VkFence                                     fence,
		uint32_t*                                   pImageIndex);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkQueuePresentKHR(
		VkQueue                                     queue,
		const VkPresentInfoKHR*                     pPresentInfo);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
	VkPhysicalDevice                            physicalDevice,
	uint32_t                                    queueFamilyIndex,
	uint32_t*                                   pCounterCount,
	VkPerformanceCounterKHR*                    pCounters,
	VkPerformanceCounterDescriptionKHR*         pCounterDescriptions);

VKAPI_ATTR void VKAPI_CALL rpi_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
	VkPhysicalDevice                            physicalDevice,
	const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
	uint32_t*                                   pNumPasses);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkAcquireProfilingLockKHR(
	VkDevice                                    device,
	const VkAcquireProfilingLockInfoKHR*        pInfo);

VKAPI_ATTR void VKAPI_CALL rpi_vkReleaseProfilingLockKHR(
	VkDevice                                    device);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetPhysicalDeviceDisplayPropertiesKHR(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkDisplayPropertiesKHR*                     pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkGetDisplayModePropertiesKHR(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	uint32_t*                                   pPropertyCount,
	VkDisplayModePropertiesKHR*                 pProperties);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDisplayModeKHR(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	const VkDisplayModeCreateInfoKHR*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDisplayModeKHR*                           pMode);

VKAPI_ATTR VkResult VKAPI_CALL rpi_vkCreateDisplayPlaneSurfaceKHR(
	VkInstance                                  instance,
	const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSurfaceKHR*                               pSurface);

#ifdef __cplusplus
}
#endif
