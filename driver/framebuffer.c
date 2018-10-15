#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateFramebuffer
 */
VkResult vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pFramebuffer);

	assert(pAllocator == 0); //TODO

	_framebuffer* fb = malloc(sizeof(_framebuffer));

	if(!fb)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	fb->renderpass = pCreateInfo->renderPass;

	fb->numAttachmentViews = pCreateInfo->attachmentCount;
	fb->attachmentViews = malloc(sizeof(_imageView) * fb->numAttachmentViews);

	if(!fb->attachmentViews)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(int c = 0; c < fb->numAttachmentViews; ++c)
	{
		memcpy(&fb->attachmentViews[c], pCreateInfo->pAttachments[c], sizeof(_imageView));
	}

	fb->width = pCreateInfo->width;
	fb->height = pCreateInfo->height;
	fb->layers = pCreateInfo->layers;

	//TODO errors/validation

	*pFramebuffer = fb;

	return VK_SUCCESS;
}

void vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(framebuffer);

	assert(pAllocator == 0); //TODO

	_framebuffer* fb = framebuffer;
	free(fb->attachmentViews);
	free(fb);
}
