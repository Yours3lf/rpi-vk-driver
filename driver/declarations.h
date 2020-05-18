#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define EXPOSE_DRIVER 0

#if EXPOSE_DRIVER == 0
	#define RPIFUNC(x) rpi_##x
#else
	#define RPIFUNC(x) x
#endif

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateInstance)(
	const VkInstanceCreateInfo*                 pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkInstance*                                 pInstance);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyInstance)(
	VkInstance                                  instance,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumeratePhysicalDevices)(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceCount,
	VkPhysicalDevice*                           pPhysicalDevices);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceFeatures)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures*                   pFeatures);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceFormatProperties)(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties*                         pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceImageFormatProperties)(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkImageTiling                               tiling,
	VkImageUsageFlags                           usage,
	VkImageCreateFlags                          flags,
	VkImageFormatProperties*                    pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceProperties)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties*                 pProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceQueueFamilyProperties)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pQueueFamilyPropertyCount,
	VkQueueFamilyProperties*                    pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceMemoryProperties)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties*           pMemoryProperties);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL RPIFUNC(vkGetInstanceProcAddr)(
	VkInstance                                  instance,
	const char*                                 pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL RPIFUNC(vkGetDeviceProcAddr)(
	VkDevice                                    device,
	const char*                                 pName);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDevice)(
	VkPhysicalDevice                            physicalDevice,
	const VkDeviceCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDevice*                                   pDevice);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDevice)(
	VkDevice                                    device,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceExtensionProperties)(
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateDeviceExtensionProperties)(
	VkPhysicalDevice                            physicalDevice,
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceLayerProperties)(
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateDeviceLayerProperties)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDeviceQueue)(
	VkDevice                                    device,
	uint32_t                                    queueFamilyIndex,
	uint32_t                                    queueIndex,
	VkQueue*                                    pQueue);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueueSubmit)(
	VkQueue                                     queue,
	uint32_t                                    submitCount,
	const VkSubmitInfo*                         pSubmits,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueueWaitIdle)(
	VkQueue                                     queue);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkDeviceWaitIdle)(
	VkDevice                                    device);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAllocateMemory)(
	VkDevice                                    device,
	const VkMemoryAllocateInfo*                 pAllocateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDeviceMemory*                             pMemory);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkFreeMemory)(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkMapMemory)(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize                                offset,
	VkDeviceSize                                size,
	VkMemoryMapFlags                            flags,
	void**                                      ppData);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkUnmapMemory)(
	VkDevice                                    device,
	VkDeviceMemory                              memory);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkFlushMappedMemoryRanges)(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkInvalidateMappedMemoryRanges)(
	VkDevice                                    device,
	uint32_t                                    memoryRangeCount,
	const VkMappedMemoryRange*                  pMemoryRanges);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDeviceMemoryCommitment)(
	VkDevice                                    device,
	VkDeviceMemory                              memory,
	VkDeviceSize*                               pCommittedMemoryInBytes);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindBufferMemory)(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindImageMemory)(
	VkDevice                                    device,
	VkImage                                     image,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetBufferMemoryRequirements)(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	VkMemoryRequirements*                       pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageMemoryRequirements)(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageSparseMemoryRequirements)(
	VkDevice                                    device,
	VkImage                                     image,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements*            pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSparseImageFormatProperties)(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkImageType                                 type,
	VkSampleCountFlagBits                       samples,
	VkImageUsageFlags                           usage,
	VkImageTiling                               tiling,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties*              pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueueBindSparse)(
	VkQueue                                     queue,
	uint32_t                                    bindInfoCount,
	const VkBindSparseInfo*                     pBindInfo,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateFence)(
	VkDevice                                    device,
	const VkFenceCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFence*                                    pFence);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyFence)(
	VkDevice                                    device,
	VkFence                                     fence,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetFences)(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetFenceStatus)(
	VkDevice                                    device,
	VkFence                                     fence);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkWaitForFences)(
	VkDevice                                    device,
	uint32_t                                    fenceCount,
	const VkFence*                              pFences,
	VkBool32                                    waitAll,
	uint64_t                                    timeout);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateSemaphore)(
	VkDevice                                    device,
	const VkSemaphoreCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSemaphore*                                pSemaphore);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySemaphore)(
	VkDevice                                    device,
	VkSemaphore                                 semaphore,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateEvent)(
	VkDevice                                    device,
	const VkEventCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkEvent*                                    pEvent);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyEvent)(
	VkDevice                                    device,
	VkEvent                                     event,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetEventStatus)(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkSetEvent)(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetEvent)(
	VkDevice                                    device,
	VkEvent                                     event);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateQueryPool)(
	VkDevice                                    device,
	const VkQueryPoolCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkQueryPool*                                pQueryPool);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyQueryPool)(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetQueryPoolResults)(
	VkDevice                                    device,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	size_t                                      dataSize,
	void*                                       pData,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateBuffer)(
	VkDevice                                    device,
	const VkBufferCreateInfo*                   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBuffer*                                   pBuffer);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyBuffer)(
	VkDevice                                    device,
	VkBuffer                                    buffer,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateBufferView)(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyBufferView)(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateImage)(
	VkDevice                                    device,
	const VkImageCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImage*                                    pImage);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyImage)(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageSubresourceLayout)(
	VkDevice                                    device,
	VkImage                                     image,
	const VkImageSubresource*                   pSubresource,
	VkSubresourceLayout*                        pLayout);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateImageView)(
	VkDevice                                    device,
	const VkImageViewCreateInfo*                pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImageView*                                pView);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyImageView)(
	VkDevice                                    device,
	VkImageView                                 imageView,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateShaderModule)(
	VkDevice                                    device,
	const VkShaderModuleCreateInfo*             pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkShaderModule*                             pShaderModule);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyShaderModule)(
	VkDevice                                    device,
	VkShaderModule                              shaderModule,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreatePipelineCache)(
	VkDevice                                    device,
	const VkPipelineCacheCreateInfo*            pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineCache*                            pPipelineCache);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyPipelineCache)(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPipelineCacheData)(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	size_t*                                     pDataSize,
	void*                                       pData);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkMergePipelineCaches)(
	VkDevice                                    device,
	VkPipelineCache                             dstCache,
	uint32_t                                    srcCacheCount,
	const VkPipelineCache*                      pSrcCaches);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateGraphicsPipelines)(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkGraphicsPipelineCreateInfo*         pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateComputePipelines)(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkComputePipelineCreateInfo*          pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyPipeline)(
	VkDevice                                    device,
	VkPipeline                                  pipeline,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreatePipelineLayout)(
	VkDevice                                    device,
	const VkPipelineLayoutCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkPipelineLayout*                           pPipelineLayout);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyPipelineLayout)(
	VkDevice                                    device,
	VkPipelineLayout                            pipelineLayout,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateSampler)(
	VkDevice                                    device,
	const VkSamplerCreateInfo*                  pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSampler*                                  pSampler);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySampler)(
	VkDevice                                    device,
	VkSampler                                   sampler,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorSetLayout)(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorSetLayout*                      pSetLayout);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorSetLayout)(
	VkDevice                                    device,
	VkDescriptorSetLayout                       descriptorSetLayout,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorPool)(
	VkDevice                                    device,
	const VkDescriptorPoolCreateInfo*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorPool*                           pDescriptorPool);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorPool)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetDescriptorPool)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	VkDescriptorPoolResetFlags                  flags);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAllocateDescriptorSets)(
	VkDevice                                    device,
	const VkDescriptorSetAllocateInfo*          pAllocateInfo,
	VkDescriptorSet*                            pDescriptorSets);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkFreeDescriptorSets)(
	VkDevice                                    device,
	VkDescriptorPool                            descriptorPool,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkUpdateDescriptorSets)(
	VkDevice                                    device,
	uint32_t                                    descriptorWriteCount,
	const VkWriteDescriptorSet*                 pDescriptorWrites,
	uint32_t                                    descriptorCopyCount,
	const VkCopyDescriptorSet*                  pDescriptorCopies);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateFramebuffer)(
	VkDevice                                    device,
	const VkFramebufferCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFramebuffer*                              pFramebuffer);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyFramebuffer)(
	VkDevice                                    device,
	VkFramebuffer                               framebuffer,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateRenderPass)(
	VkDevice                                    device,
	const VkRenderPassCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkRenderPass*                               pRenderPass);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyRenderPass)(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetRenderAreaGranularity)(
	VkDevice                                    device,
	VkRenderPass                                renderPass,
	VkExtent2D*                                 pGranularity);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateCommandPool)(
	VkDevice                                    device,
	const VkCommandPoolCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkCommandPool*                              pCommandPool);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyCommandPool)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetCommandPool)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolResetFlags                     flags);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAllocateCommandBuffers)(
	VkDevice                                    device,
	const VkCommandBufferAllocateInfo*          pAllocateInfo,
	VkCommandBuffer*                            pCommandBuffers);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkFreeCommandBuffers)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBeginCommandBuffer)(
	VkCommandBuffer                             commandBuffer,
	const VkCommandBufferBeginInfo*             pBeginInfo);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEndCommandBuffer)(
	VkCommandBuffer                             commandBuffer);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkResetCommandBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkCommandBufferResetFlags                   flags);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindPipeline)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipeline                                  pipeline);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetViewport)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstViewport,
	uint32_t                                    viewportCount,
	const VkViewport*                           pViewports);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetScissor)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstScissor,
	uint32_t                                    scissorCount,
	const VkRect2D*                             pScissors);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetLineWidth)(
	VkCommandBuffer                             commandBuffer,
	float                                       lineWidth);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDepthBias)(
	VkCommandBuffer                             commandBuffer,
	float                                       depthBiasConstantFactor,
	float                                       depthBiasClamp,
	float                                       depthBiasSlopeFactor);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetBlendConstants)(
	VkCommandBuffer                             commandBuffer,
	const float                                 blendConstants[4]);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDepthBounds)(
	VkCommandBuffer                             commandBuffer,
	float                                       minDepthBounds,
	float                                       maxDepthBounds);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilCompareMask)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    compareMask);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilWriteMask)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    writeMask);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilReference)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    reference);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindDescriptorSets)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineBindPoint                         pipelineBindPoint,
	VkPipelineLayout                            layout,
	uint32_t                                    firstSet,
	uint32_t                                    descriptorSetCount,
	const VkDescriptorSet*                      pDescriptorSets,
	uint32_t                                    dynamicOffsetCount,
	const uint32_t*                             pDynamicOffsets);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindIndexBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	VkIndexType                                 indexType);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindVertexBuffers)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstBinding,
	uint32_t                                    bindingCount,
	const VkBuffer*                             pBuffers,
	const VkDeviceSize*                         pOffsets);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDraw)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    vertexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstVertex,
	uint32_t                                    firstInstance);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDrawIndexed)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    indexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstIndex,
	int32_t                                     vertexOffset,
	uint32_t                                    firstInstance);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDrawIndirect)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDrawIndexedIndirect)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDispatch)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDispatchIndirect)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdCopyBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferCopy*                         pRegions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdCopyImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageCopy*                          pRegions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBlitImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageBlit*                          pRegions,
	VkFilter                                    filter);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdCopyBufferToImage)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdCopyImageToBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferImageCopy*                    pRegions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdUpdateBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                dataSize,
	const void*                                 pData);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdFillBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                size,
	uint32_t                                    data);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearColorImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearColorValue*                    pColor,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearDepthStencilImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearDepthStencilValue*             pDepthStencil,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearAttachments)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    attachmentCount,
	const VkClearAttachment*                    pAttachments,
	uint32_t                                    rectCount,
	const VkClearRect*                          pRects);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdResolveImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     srcImage,
	VkImageLayout                               srcImageLayout,
	VkImage                                     dstImage,
	VkImageLayout                               dstImageLayout,
	uint32_t                                    regionCount,
	const VkImageResolve*                       pRegions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetEvent)(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdResetEvent)(
	VkCommandBuffer                             commandBuffer,
	VkEvent                                     event,
	VkPipelineStageFlags                        stageMask);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdWaitEvents)(
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

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdPipelineBarrier)(
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

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBeginQuery)(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query,
	VkQueryControlFlags                         flags);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdEndQuery)(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    query);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdResetQueryPool)(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdWriteTimestamp)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineStageFlagBits                     pipelineStage,
	VkQueryPool                                 queryPool,
	uint32_t                                    query);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdCopyQueryPoolResults)(
	VkCommandBuffer                             commandBuffer,
	VkQueryPool                                 queryPool,
	uint32_t                                    firstQuery,
	uint32_t                                    queryCount,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                stride,
	VkQueryResultFlags                          flags);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdPushConstants)(
	VkCommandBuffer                             commandBuffer,
	VkPipelineLayout                            layout,
	VkShaderStageFlags                          stageFlags,
	uint32_t                                    offset,
	uint32_t                                    size,
	const void*                                 pValues);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBeginRenderPass)(
	VkCommandBuffer                             commandBuffer,
	const VkRenderPassBeginInfo*                pRenderPassBegin,
	VkSubpassContents                           contents);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdNextSubpass)(
	VkCommandBuffer                             commandBuffer,
	VkSubpassContents                           contents);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdEndRenderPass)(
	VkCommandBuffer                             commandBuffer);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdExecuteCommands)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    commandBufferCount,
	const VkCommandBuffer*                      pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumerateInstanceVersion)(
	uint32_t*                                   pApiVersion);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindBufferMemory2)(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindBufferMemoryInfo*               pBindInfos);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkBindImageMemory2)(
	VkDevice                                    device,
	uint32_t                                    bindInfoCount,
	const VkBindImageMemoryInfo*                pBindInfos);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDeviceGroupPeerMemoryFeatures)(
	VkDevice                                    device,
	uint32_t                                    heapIndex,
	uint32_t                                    localDeviceIndex,
	uint32_t                                    remoteDeviceIndex,
	VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDeviceMask)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    deviceMask);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdDispatchBase)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    baseGroupX,
	uint32_t                                    baseGroupY,
	uint32_t                                    baseGroupZ,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumeratePhysicalDeviceGroups)(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceGroupCount,
	VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageMemoryRequirements2)(
	VkDevice                                    device,
	const VkImageMemoryRequirementsInfo2*       pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetBufferMemoryRequirements2)(
	VkDevice                                    device,
	const VkBufferMemoryRequirementsInfo2*      pInfo,
	VkMemoryRequirements2*                      pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetImageSparseMemoryRequirements2)(
	VkDevice                                    device,
	const VkImageSparseMemoryRequirementsInfo2* pInfo,
	uint32_t*                                   pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceFeatures2)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceFeatures2*                  pFeatures);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceProperties2)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceProperties2*                pProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceFormatProperties2)(
	VkPhysicalDevice                            physicalDevice,
	VkFormat                                    format,
	VkFormatProperties2*                        pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceImageFormatProperties2)(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
	VkImageFormatProperties2*                   pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceQueueFamilyProperties2)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pQueueFamilyPropertyCount,
	VkQueueFamilyProperties2*                   pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceMemoryProperties2)(
	VkPhysicalDevice                            physicalDevice,
	VkPhysicalDeviceMemoryProperties2*          pMemoryProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSparseImageFormatProperties2)(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
	uint32_t*                                   pPropertyCount,
	VkSparseImageFormatProperties2*             pProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkTrimCommandPool)(
	VkDevice                                    device,
	VkCommandPool                               commandPool,
	VkCommandPoolTrimFlags                      flags);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDeviceQueue2)(
	VkDevice                                    device,
	const VkDeviceQueueInfo2*                   pQueueInfo,
	VkQueue*                                    pQueue);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateSamplerYcbcrConversion)(
	VkDevice                                    device,
	const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSamplerYcbcrConversion*                   pYcbcrConversion);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySamplerYcbcrConversion)(
	VkDevice                                    device,
	VkSamplerYcbcrConversion                    ycbcrConversion,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDescriptorUpdateTemplate)(
	VkDevice                                    device,
	const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroyDescriptorUpdateTemplate)(
	VkDevice                                    device,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkUpdateDescriptorSetWithTemplate)(
	VkDevice                                    device,
	VkDescriptorSet                             descriptorSet,
	VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
	const void*                                 pData);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceExternalBufferProperties)(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
	VkExternalBufferProperties*                 pExternalBufferProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceExternalFenceProperties)(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
	VkExternalFenceProperties*                  pExternalFenceProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceExternalSemaphoreProperties)(
	VkPhysicalDevice                            physicalDevice,
	const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
	VkExternalSemaphoreProperties*              pExternalSemaphoreProperties);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetDescriptorSetLayoutSupport)(
	VkDevice                                    device,
	const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
	VkDescriptorSetLayoutSupport*               pSupport);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySurfaceKHR)(
		VkInstance                                  instance,
		VkSurfaceKHR                                surface,
		const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateSwapchainKHR)(
		VkDevice                                    device,
		const VkSwapchainCreateInfoKHR*             pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSwapchainKHR*                             pSwapchain);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceSupportKHR)(
		VkPhysicalDevice                            physicalDevice,
		uint32_t                                    queueFamilyIndex,
		VkSurfaceKHR                                surface,
		VkBool32*                                   pSupported);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pSurfaceFormatCount,
		VkSurfaceFormatKHR*                         pSurfaceFormats);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)(
		VkPhysicalDevice                            physicalDevice,
		VkSurfaceKHR                                surface,
		uint32_t*                                   pPresentModeCount,
		VkPresentModeKHR*                           pPresentModes);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetSwapchainImagesKHR)(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint32_t*                                   pSwapchainImageCount,
		VkImage*                                    pSwapchainImages);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAcquireNextImageKHR)(
		VkDevice                                    device,
		VkSwapchainKHR                              swapchain,
		uint64_t                                    timeout,
		VkSemaphore                                 semaphore,
		VkFence                                     fence,
		uint32_t*                                   pImageIndex);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkQueuePresentKHR)(
		VkQueue                                     queue,
		const VkPresentInfoKHR*                     pPresentInfo);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t                                    queueFamilyIndex,
	uint32_t*                                   pCounterCount,
	VkPerformanceCounterKHR*                    pCounters,
	VkPerformanceCounterDescriptionKHR*         pCounterDescriptions);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)(
	VkPhysicalDevice                            physicalDevice,
	const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
	uint32_t*                                   pNumPasses);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkAcquireProfilingLockKHR)(
	VkDevice                                    device,
	const VkAcquireProfilingLockInfoKHR*        pInfo);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkReleaseProfilingLockKHR)(
	VkDevice                                    device);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceDisplayPropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkDisplayPropertiesKHR*                     pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetDisplayModePropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	uint32_t*                                   pPropertyCount,
	VkDisplayModePropertiesKHR*                 pProperties);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDisplayModeKHR)(
	VkPhysicalDevice                            physicalDevice,
	VkDisplayKHR                                display,
	const VkDisplayModeCreateInfoKHR*           pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkDisplayModeKHR*                           pMode);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkCreateDisplayPlaneSurfaceKHR)(
	VkInstance                                  instance,
	const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSurfaceKHR*                               pSurface);

VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkDestroySwapchainKHR)(
	VkDevice                                    device,
	VkSwapchainKHR                              swapchain,
	const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetDisplayPlaneSupportedDisplaysKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t                                    planeIndex,
	uint32_t*                                   pDisplayCount,
	VkDisplayKHR*                               pDisplays);

VKAPI_ATTR VkResult VKAPI_CALL RPIFUNC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR)(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkDisplayPlanePropertiesKHR*                pProperties);

#ifdef __cplusplus
}
#endif
