#include "common.h"

#include "kernel/vc4_packet.h"

#include "QPUassembler/qpu_assembler.h"

//TODO collect shader performance data
//eg number of texture samples etc.
//TODO check if shader has flow control and make sure instance also has flow control
//TODO make sure instance has threaded fs if shader contains thread switch

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

	shader->hasThreadSwitch = 0;

	for(int c = 0; c < RPI_ASSEMBLY_TYPE_MAX; ++c)
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

			//need to create a temporary copy as the assembly algorithm is destructive
			uint32_t stringLength = strlen(pCreateInfo->asmStrings[c]);
			char* tmpShaderStr = ALLOCATE(stringLength+1, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			memcpy(tmpShaderStr, pCreateInfo->asmStrings[c], stringLength+1);

			assemble_qpu_asm(tmpShaderStr, instructions);

			FREE(tmpShaderStr);

			shader->bos[c] = vc4_bo_alloc_shader(controlFd, instructions, &size);

			//TODO if debug...
			for(uint64_t c = 0; c < numInstructions; ++c)
			{
				printf("%#llx ", instructions[c]);
				disassemble_qpu_asm(instructions[c]);
			}

			for(uint64_t c = 0; c < numInstructions; ++c)
			{
				if((instructions[c] & (0xf << 60)) == (2 << 60))
				{
					shader->hasThreadSwitch = 1;
					break;
				}
			}

			printf("\n");

			FREE(instructions);

			shader->sizes[c] = size;
		}
		else
		{
			shader->bos[c] = 0;
			shader->sizes[c] = 0;
		}
	}

	shader->numMappings = pCreateInfo->numMappings;

	if(pCreateInfo->numMappings > 0)
	{
		shader->mappings = ALLOCATE(sizeof(VkRpiAssemblyMappingEXT)*pCreateInfo->numMappings, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

		if(!shader->mappings)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(shader->mappings, pCreateInfo->mappings, sizeof(VkRpiAssemblyMappingEXT)*pCreateInfo->numMappings);
	}

	*pShaderModule = shader;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateShaderModule
 */
VkResult vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	UNSUPPORTED(vkCreateShaderModule);
	return VK_SUCCESS;
}

void vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
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
