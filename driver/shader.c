#include "common.h"

#include "kernel/vc4_packet.h"

#include "QPUassembler/qpu_assembler.h"

VkResult vkCreateShaderModuleFromRpiAssemblyEXT(VkDevice device, VkRpiShaderModuleAssemblyCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	assert(device);
	assert(pCreateInfo);
	assert(pShaderModule);
	assert(pCreateInfo->asmStrings);

	_shaderModule* shader = ALLOCATE(sizeof(_shaderModule), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!shader)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	for(int c = 0; c < VK_RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(pCreateInfo->asmStrings[c])
		{
			uint32_t numInstructions = get_num_instructions(pCreateInfo->asmStrings[c]);
			uint32_t size = sizeof(uint64_t)*numInstructions;
			//TODO this alloc feels kinda useless, we just copy the data anyway to kernel space
			//why not map kernel space mem to user space instead?
			uint64_t* instructions = ALLOCATE(size, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!instructions)
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}
			assemble_qpu_asm(pCreateInfo->asmStrings[c], instructions);
			shader->bos[c] = vc4_bo_alloc_shader(controlFd, instructions, &size);

			//TODO if debug...
			for(uint64_t c = 0; c < numInstructions; ++c)
			{
				printf("%#llx ", instructions[c]);
				disassemble_qpu_asm(instructions[c]);
			}

			printf("\n");

			FREE(instructions);

			shader->sizes[c] = size;

			shader->numDescriptorBindings[c] = pCreateInfo->numDescriptorBindings[c];

			if(pCreateInfo->numDescriptorBindings[c] > 0)
			{
				shader->descriptorBindings[c] = ALLOCATE(sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
				if(!shader->descriptorBindings[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
				memcpy(shader->descriptorBindings[c], pCreateInfo->descriptorBindings + (c*pCreateInfo->numDescriptorBindings[c]), sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c]);

				shader->descriptorSets[c] = ALLOCATE(sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
				if(!shader->descriptorSets[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
				memcpy(shader->descriptorSets[c], pCreateInfo->descriptorSets + (c*pCreateInfo->numDescriptorBindings[c]), sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c]);

				shader->descriptorTypes[c] = ALLOCATE(sizeof(VkDescriptorType)*pCreateInfo->numDescriptorBindings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
				if(!shader->descriptorTypes[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
				memcpy(shader->descriptorTypes[c], pCreateInfo->descriptorTypes + (c*pCreateInfo->numDescriptorBindings[c]), sizeof(VkDescriptorType)*pCreateInfo->numDescriptorBindings[c]);

				shader->descriptorCounts[c] = ALLOCATE(sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
				if(!shader->descriptorCounts[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
				memcpy(shader->descriptorCounts[c], pCreateInfo->descriptorCounts + (c*pCreateInfo->numDescriptorBindings[c]), sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c]);

				shader->descriptorArrayElems[c] = ALLOCATE(sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
				if(!shader->descriptorArrayElems[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
				memcpy(shader->descriptorArrayElems[c], pCreateInfo->descriptorArrayElems + (c*pCreateInfo->numDescriptorBindings[c]), sizeof(uint32_t)*pCreateInfo->numDescriptorBindings[c]);
			}
		}
		else
		{
			shader->bos[c] = 0;
			shader->descriptorBindings[c] = 0;
			shader->descriptorCounts[c] = 0;
			shader->descriptorSets[c] = 0;
			shader->descriptorTypes[c] = 0;
			shader->descriptorArrayElems[c] = 0;
			shader->numDescriptorBindings[c] = 0;
			shader->sizes[c] = 0;
		}
	}

	*pShaderModule = shader;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateShaderModule
 */
VkResult vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	//TODO
	return VK_SUCCESS;
}

void vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
	assert(device);

	_shaderModule* shader = shaderModule;

	if(shader)
	{
		for(int c = 0; c < VK_RPI_ASSEMBLY_TYPE_MAX; ++c)
		{
			if(shader->bos[c])
			{
				vc4_bo_free(controlFd, shader->bos[c], 0, shader->sizes[c]);
				if(shader->numDescriptorBindings[c]>0)
				{
					FREE(shader->descriptorBindings[c]);
					FREE(shader->descriptorSets[c]);
					FREE(shader->descriptorTypes[c]);
					FREE(shader->descriptorCounts[c]);
					FREE(shader->descriptorArrayElems[c]);
				}
			}
		}
		FREE(shader);
	}
}
