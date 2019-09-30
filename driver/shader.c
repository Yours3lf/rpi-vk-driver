#include "common.h"

#include "kernel/vc4_packet.h"

#include "QPUassembler/qpu_assembler.h"

#include "vkExt.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateShaderModule
 */
VkResult rpi_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	UNSUPPORTED(rpi_vkCreateShaderModule);
	return VK_SUCCESS;
}

void rpi_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_shaderModule* shader = shaderModule;

	if(shader)
	{
		for(int c = 0; c < RPI_ASSEMBLY_TYPE_MAX; ++c)
		{
			if(shader->bos[c])
			{
				vc4_bo_free(controlFd, shader->bos[c], 0, shader->sizes[c]);
			}
		}

		if(shader->numMappings>0)
		{
			FREE(shader->mappings);
		}

		FREE(shader);
	}
}
