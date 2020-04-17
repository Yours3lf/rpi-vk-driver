#include "common.h"

#include "kernel/vc4_packet.h"

#include "QPUassembler/qpu_assembler.h"

#include "vkExt.h"

//TODO collect shader performance data
//eg number of texture samples etc.
//TODO check if shader has flow control and make sure instance also has flow control
//TODO make sure instance has threaded fs if shader contains thread switch

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateShaderModule
 */
VkResult rpi_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	uint32_t magic = pCreateInfo->pCode[2];
	VkRpiShaderModuleAssemblyCreateInfoEXT* ci = pCreateInfo->pCode[4];

	//shader magic doesn't add up
	if(magic != 0x14E45250)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	assert(ci);
	assert(pShaderModule);
	assert(ci->instructions);

	_shaderModule* shader = ALLOCATE(sizeof(_shaderModule), 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

	if(!shader)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	shader->hasThreadSwitch = 0;

	for(int c = 0; c < RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(ci->instructions[c])
		{
			for(uint64_t d = 0; d < ci->numInstructions[c]; ++d)
			{
				uint64_t s = (ci->instructions[c][d] & (0xfll << 60)) >> 60;
				if(s == 2ll)
				{
					shader->hasThreadSwitch = 1;
					break;
				}
			}

			shader->numVaryings = 0;
			for(uint64_t d = 0; d < ci->numInstructions[c]; ++d)
			{
				unsigned is_sem = ((ci->instructions[c][d] & (0x7fll << 57)) >> 57) == 0x74;
				unsigned sig_bits = ((ci->instructions[c][d] & (0xfll << 60)) >> 60);

				//if it's an ALU instruction
				if(!is_sem && sig_bits != 14 && sig_bits != 15)
				{
					unsigned raddr_a = ((ci->instructions[c][d] & (0x3fll << 18)) >> 18);
					unsigned raddr_b = ((ci->instructions[c][d] & (0x3fll << 12)) >> 12);

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

			shader->sizes[c] = ci->numInstructions[c]*sizeof(uint64_t);


			for(uint64_t e = 0; e < shader->sizes[c] / 8; ++e)
			{
				printf("%#llx ", ci->instructions[c][e]);
				disassemble_qpu_asm(ci->instructions[c][e]);
			}
			printf("\n");
			shader->bos[c] = vc4_bo_alloc_shader(controlFd, ci->instructions[c], &shader->sizes[c]);
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

	*pShaderModule = shader;

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
