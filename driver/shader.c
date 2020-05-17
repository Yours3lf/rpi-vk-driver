#include "common.h"

#include "kernel/vc4_packet.h"

#include "QPUassembler/qpu_assembler.h"
#include "QPUassembler/vc4_qpu_enums.h"
#include "QPUassembler/vc4_qpu_defines.h"

#include "vkExt.h"

//TODO check if shader has flow control and make sure instance also has flow control

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
	shader->numTextureSamples = 0;
	shader->numVaryings = 0;
	shader->numFragUniformReads = 0;
	shader->numVertUniformReads = 0;
	shader->numCoordUniformReads = 0;
	shader->numVertVPMreads = 0;
	shader->numCoordVPMreads = 0;
	shader->numVertVPMwrites = 0;
	shader->numCoordVPMwrites = 0;
	shader->numFragCycles = 0;
	shader->numVertCycles = 0;
	shader->numCoordCycles = 0;
	shader->numFragALUcycles = 0;
	shader->numVertALUcycles = 0;
	shader->numCoordALUcycles = 0;
	shader->numEmptyFragALUinstructions = 0;
	shader->numEmptyVertALUinstructions = 0;
	shader->numEmptyCoordALUinstructions = 0;
	shader->numFragBranches = 0;
	shader->numVertBranches = 0;
	shader->numCoordBranches = 0;
	shader->numFragSFUoperations = 0;
	shader->numVertSFUoperations = 0;
	shader->numCoordSFUoperations = 0;

	uint32_t hadVertex = 0, hadCoordinate = 0;

	for(int c = 0; c < VK_RPI_ASSEMBLY_TYPE_MAX; ++c)
	{
		if(ci->instructions[c])
		{
			if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
			{
				hadVertex = 1;
			}
			else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
			{
				hadCoordinate = 1;
			}

			if(c == VK_RPI_ASSEMBLY_TYPE_FRAGMENT)
			{
				for(uint64_t d = 0; d < ci->numInstructions[c]; ++d)
				{
					uint64_t s = (ci->instructions[c][d] & (0xfll << QPU_SIG_SHIFT)) >> QPU_SIG_SHIFT;
					if(s == QPU_SIG_THREAD_SWITCH || s == QPU_SIG_LAST_THREAD_SWITCH)
					{
						shader->hasThreadSwitch = 1;
						break;
					}
				}

				uint32_t numTMUwrites = 0;
				for(uint64_t d = 0; d < ci->numInstructions[c]; ++d)
				{
					unsigned is_sem = ((ci->instructions[c][d] & (0x7fll << 57)) >> 57) == 0x74;
					unsigned sig_bits = ((ci->instructions[c][d] & (0xfll << QPU_SIG_SHIFT)) >> QPU_SIG_SHIFT);

					unsigned waddr_add = ((ci->instructions[c][d] & (0x3fll << QPU_WADDR_ADD_SHIFT)) >> QPU_WADDR_ADD_SHIFT);
					unsigned waddr_mul = ((ci->instructions[c][d] & (0x3fll << QPU_WADDR_MUL_SHIFT)) >> QPU_WADDR_MUL_SHIFT);

					if(waddr_add == QPU_W_SFU_RECIP ||
					   waddr_add == QPU_W_SFU_RECIPSQRT ||
					   waddr_add == QPU_W_SFU_EXP ||
					   waddr_add == QPU_W_SFU_LOG)
					{
						shader->numFragSFUoperations++;
					}

					if(waddr_mul == QPU_W_SFU_RECIP ||
					   waddr_mul == QPU_W_SFU_RECIPSQRT ||
					   waddr_mul == QPU_W_SFU_EXP ||
					   waddr_mul == QPU_W_SFU_LOG)
					{
						shader->numFragSFUoperations++;
					}

					if(waddr_add == QPU_W_TMU0_S ||
					   waddr_add == QPU_W_TMU0_T ||
					   waddr_add == QPU_W_TMU0_R ||
					   waddr_add == QPU_W_TMU0_B ||
					   waddr_add == QPU_W_TMU1_S ||
					   waddr_add == QPU_W_TMU1_T ||
					   waddr_add == QPU_W_TMU1_R ||
					   waddr_add == QPU_W_TMU1_B)
					{
						numTMUwrites++;
					}

					if(waddr_mul == QPU_W_TMU0_S ||
					   waddr_mul == QPU_W_TMU0_T ||
					  waddr_mul == QPU_W_TMU0_R ||
					  waddr_mul == QPU_W_TMU0_B ||
					  waddr_mul == QPU_W_TMU1_S ||
					  waddr_mul == QPU_W_TMU1_T ||
					  waddr_mul == QPU_W_TMU1_R ||
					  waddr_mul == QPU_W_TMU1_B
					   )
					{
						numTMUwrites++;
					}

					if(!is_sem && (sig_bits == QPU_SIG_LOAD_TMU0 || sig_bits == QPU_SIG_LOAD_TMU1))
					{
						shader->numTextureSamples++;
						shader->numFragUniformReads++;
					}

					//if it's an ALU instruction
					if(!is_sem && sig_bits != QPU_SIG_LOAD_IMM && sig_bits != QPU_SIG_BRANCH)
					{
						shader->numFragALUcycles++;

						if(waddr_add == QPU_W_NOP)
						{
							shader->numEmptyFragALUinstructions++;
						}

						if(waddr_mul == QPU_W_NOP)
						{
							shader->numEmptyFragALUinstructions++;
						}

						unsigned raddr_a = ((ci->instructions[c][d] & (0x3fll << QPU_RADDR_A_SHIFT)) >> QPU_RADDR_A_SHIFT);
						unsigned raddr_b = ((ci->instructions[c][d] & (0x3fll << QPU_RADDR_B_SHIFT)) >> QPU_RADDR_B_SHIFT);

						if(raddr_a == QPU_R_VARY)
						{
							shader->numVaryings++;
						}
						else if(raddr_a == QPU_R_UNIF)
						{
							shader->numFragUniformReads++;
						}

						//don't count small immediates
						if(sig_bits != QPU_SIG_SMALL_IMM && raddr_b == QPU_R_VARY)
						{
							shader->numVaryings++;
						}
						else if(sig_bits != QPU_SIG_SMALL_IMM && raddr_b == QPU_R_UNIF)
						{
							shader->numFragUniformReads++;
						}
					}
					else if(!is_sem && sig_bits == QPU_SIG_BRANCH)
					{
						shader->numFragBranches++;
					}
				}

				if(numTMUwrites > 1)
				{
					shader->numFragUniformReads += numTMUwrites;
				}
			}

			if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX || c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
			{
				for(uint64_t d = 0; d < ci->numInstructions[c]; ++d)
				{
					unsigned waddr_add = ((ci->instructions[c][d] & (0x3fll << QPU_WADDR_ADD_SHIFT)) >> QPU_WADDR_ADD_SHIFT);
					unsigned waddr_mul = ((ci->instructions[c][d] & (0x3fll << QPU_WADDR_MUL_SHIFT)) >> QPU_WADDR_MUL_SHIFT);

					unsigned is_sem = ((ci->instructions[c][d] & (0x7fll << 57)) >> 57) == 0x74;
					unsigned sig_bits = ((ci->instructions[c][d] & (0xfll << QPU_SIG_SHIFT)) >> QPU_SIG_SHIFT);

					if(waddr_add == QPU_W_SFU_RECIP ||
					   waddr_add == QPU_W_SFU_RECIPSQRT ||
					   waddr_add == QPU_W_SFU_EXP ||
					   waddr_add == QPU_W_SFU_LOG)
					{
						if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
						{
							shader->numVertSFUoperations++;
						}
						else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
						{
							shader->numCoordSFUoperations++;
						}
					}

					if(waddr_mul == QPU_W_SFU_RECIP ||
					   waddr_mul == QPU_W_SFU_RECIPSQRT ||
					   waddr_mul == QPU_W_SFU_EXP ||
					   waddr_mul == QPU_W_SFU_LOG)
					{
						if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
						{
							shader->numVertSFUoperations++;
						}
						else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
						{
							shader->numCoordSFUoperations++;
						}
					}

					if(waddr_add == QPU_W_VPM || waddr_mul == QPU_W_VPM)
					{
						if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
						{
							shader->numVertVPMwrites++;
						}
						else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
						{
							shader->numCoordVPMwrites++;
						}
					}

					//if it's an ALU instruction
					if(!is_sem && sig_bits != QPU_SIG_LOAD_IMM && sig_bits != QPU_SIG_BRANCH)
					{
						if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
						{
							shader->numVertALUcycles++;
						}
						else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
						{
							shader->numCoordALUcycles++;
						}

						if(waddr_add == QPU_W_NOP)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numEmptyVertALUinstructions++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numEmptyCoordALUinstructions++;
							}
						}

						if(waddr_mul == QPU_W_NOP)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numEmptyVertALUinstructions++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numEmptyCoordALUinstructions++;
							}
						}

						unsigned raddr_a = ((ci->instructions[c][d] & (0x3fll << QPU_RADDR_A_SHIFT)) >> QPU_RADDR_A_SHIFT);
						unsigned raddr_b = ((ci->instructions[c][d] & (0x3fll << QPU_RADDR_B_SHIFT)) >> QPU_RADDR_B_SHIFT);

						if(raddr_a == QPU_R_VPM)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numVertVPMreads++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numCoordVPMreads++;
							}
						}
						else if(raddr_a == QPU_R_UNIF)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numVertUniformReads++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numCoordUniformReads++;
							}
						}

						//don't count small immediates
						if(sig_bits != QPU_SIG_SMALL_IMM && raddr_b == QPU_R_VPM)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numVertVPMreads++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numCoordVPMreads++;
							}
						}
						else if(sig_bits != QPU_SIG_SMALL_IMM && raddr_b == QPU_R_UNIF)
						{
							if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
							{
								shader->numVertUniformReads++;
							}
							else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
							{
								shader->numCoordUniformReads++;
							}
						}
					}
					else if(!is_sem && sig_bits == QPU_SIG_BRANCH)
					{
						if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
						{
							shader->numVertBranches++;
						}
						else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
						{
							shader->numCoordBranches++;
						}
					}
				}
			}

			if(c == VK_RPI_ASSEMBLY_TYPE_FRAGMENT)
			{
				shader->numFragCycles = ci->numInstructions[c];
			}
			else if(c == VK_RPI_ASSEMBLY_TYPE_VERTEX)
			{
				shader->numVertCycles = ci->numInstructions[c];
			}
			else if(c == VK_RPI_ASSEMBLY_TYPE_COORDINATE)
			{
				shader->numCoordCycles = ci->numInstructions[c];
			}

			shader->sizes[c] = ci->numInstructions[c]*sizeof(uint64_t);

			/**
			for(uint64_t e = 0; e < shader->sizes[c] / 8; ++e)
			{
				printf("%#llx ", ci->instructions[c][e]);
				disassemble_qpu_asm(ci->instructions[c][e]);
			}
			printf("\n");
			/**/

			shader->bos[c] = vc4_bo_alloc_shader(controlFd, ci->instructions[c], &shader->sizes[c]);
			assert(shader->bos[c]);
		}
		else
		{
			shader->bos[c] = 0;
			shader->sizes[c] = 0;
		}

		if(ci->numMappings)
		{
			shader->numMappings[c] = ci->numMappings[c];

			if(ci->numMappings[c] > 0)
			{
				shader->mappings[c] = ALLOCATE(sizeof(VkRpiAssemblyMappingEXT)*ci->numMappings[c], 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

				if(!shader->mappings[c])
				{
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}

				memcpy(shader->mappings[c], ci->mappings[c], sizeof(VkRpiAssemblyMappingEXT)*ci->numMappings[c]);
			}
		}
		else
		{
			shader->numMappings[c] = 0;
			shader->mappings[c] = 0;
		}
	}

	assert(hadVertex == hadCoordinate);

	*pShaderModule = shader;

	return VK_SUCCESS;
}

void rpi_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
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
			}

			if(shader->numMappings[c]>0)
			{
				FREE(shader->mappings[c]);
			}
		}

		FREE(shader);
	}
}
