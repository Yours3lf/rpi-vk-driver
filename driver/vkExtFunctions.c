#pragma once

#include "common.h"
#include "QPUassembler/qpu_assembler.h"
#include "modeset.h"
#include "vkExtFunctions.h"

#ifdef __cplusplus
extern "C" {
#endif

//TODO collect shader performance data
//eg number of texture samples etc.
//TODO check if shader has flow control and make sure instance also has flow control
//TODO make sure instance has threaded fs if shader contains thread switch

VkResult rpi_vkCreateShaderModuleFromRpiAssemblyEXT(VkPhysicalDevice physicalDevice)
{
	assert(physicalDevice);

	_physicalDevice* ptr = physicalDevice;
	VkRpiShaderModuleAssemblyCreateInfoEXT* ci = ptr->customData;
	const const VkAllocationCallbacks* pAllocator = ci->pAllocator;

	assert(ci);
	assert(ci->pShaderModule);
	assert(ci->asmStrings);

	_shaderModule* shader = ALLOCATE(sizeof(_shaderModule), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!shader)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	shader->hasThreadSwitch = 0;

	for(int c = 0; c < RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(ci->asmStrings[c])
		{
			uint32_t numInstructions = get_num_instructions(ci->asmStrings[c]);
			uint32_t size = sizeof(uint64_t)*numInstructions;
			//TODO this alloc feels kinda useless, we just copy the data anyway to kernel space
			//why not map kernel space mem to user space instead?
			shader->instructions[c] = ALLOCATE(size, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			if(!shader->instructions[c])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			//need to create a temporary copy as the assembly algorithm is destructive
			uint32_t stringLength = strlen(ci->asmStrings[c]);
			char* tmpShaderStr = ALLOCATE(stringLength+1, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
			memcpy(tmpShaderStr, ci->asmStrings[c], stringLength+1);

			assemble_qpu_asm(tmpShaderStr, shader->instructions[c]);

			FREE(tmpShaderStr);

			for(uint64_t d = 0; d < numInstructions; ++d)
			{
				uint64_t s = (shader->instructions[c][d] & (0xfll << 60)) >> 60;
				if(s == 2ll)
				{
					shader->hasThreadSwitch = 1;
					break;
				}
			}

			shader->numVaryings = 0;
			for(uint64_t d = 0; d < numInstructions; ++d)
			{
				unsigned is_sem = ((shader->instructions[c][d] & (0x7fll << 57)) >> 57) == 0x74;
				unsigned sig_bits = ((shader->instructions[c][d] & (0xfll << 60)) >> 60);

				//if it's an ALU instruction
				if(!is_sem && sig_bits != 14 && sig_bits != 15)
				{
					unsigned raddr_a = ((shader->instructions[c][d] & (0x3fll << 18)) >> 18);
					unsigned raddr_b = ((shader->instructions[c][d] & (0x3fll << 12)) >> 12);

					if(raddr_a == 35)
					{
						shader->numVaryings++;
					}

					//don't count small immediates
					if(sig_bits != 13 && raddr_b == 35)
					{
						shader->numVaryings++;
					}
				}
			}

			shader->sizes[c] = size;


			for(uint64_t e = 0; e < shader->sizes[c] / 8; ++e)
			{
				printf("%#llx ", shader->instructions[c][e]);
				disassemble_qpu_asm(shader->instructions[c][e]);
			}
			printf("\n");
			shader->bos[c] = vc4_bo_alloc_shader(controlFd, shader->instructions[c], &shader->sizes[c]);
		}
		else
		{
			shader->bos[c] = 0;
			shader->sizes[c] = 0;
		}
	}

	shader->numMappings = ci->numMappings;

	if(ci->numMappings > 0)
	{
		shader->mappings = ALLOCATE(sizeof(VkRpiAssemblyMappingEXT)*ci->numMappings, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

		if(!shader->mappings)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		memcpy(shader->mappings, ci->mappings, sizeof(VkRpiAssemblyMappingEXT)*ci->numMappings);
	}

	*ci->pShaderModule = shader;

	return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif
