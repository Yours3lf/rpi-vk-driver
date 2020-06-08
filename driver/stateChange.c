#include "common.h"

#include "kernel/vc4_packet.h"

#include "declarations.h"

#include "QPUassembler/qpu_assembler.h"

void createClearShaderModule(VkDevice device, VkShaderModule* blitShaderModule, VkShaderModule* blitShaderModuleNoColor)
{
	char vs_asm_code[] =
			///0x40000000 = 2.0
			///uni = 1.0
			///rb0 = 2 - 1 = 1
			"sig_small_imm ; rx0 = fsub.ws.always(b, a, uni, 0x40000000) ; nop = nop(r0, r0) ;\n"
			///set up VPM read for subsequent reads
			///0x00201a00: 0000 0000 0010 0000 0001 1010 0000 0000
			///addr: 0
			///size: 32bit
			///packed
			///horizontal
			///stride=1
			///vectors to read = 2 (how many components)
			"sig_load_imm ; vr_setup = load32.always(0x00201a00) ; nop = load32.always() ;\n"
			///uni = viewportXScale
			///r0 = vpm * uni
			"sig_none ; nop = nop(r0, r0, vpm_read, uni) ; r0 = fmul.always(a, b) ;\n"
			///r1 = r0 * rb0 (1)
			"sig_none ; nop = nop(r0, r0, nop, rb0) ; r1 = fmul.always(r0, b) ;\n"
			///uni = viewportYScale
			///ra0.16a = int(r1), r2 = vpm * uni
			"sig_none ; rx0.16a = ftoi.always(r1, r1, vpm_read, uni) ; r2 = fmul.always(a, b) ;\n"
			///r3 = r2 * rb0
			"sig_none ; nop = nop(r0, r0, nop, rb0) ; r3 = fmul.always(r2, b) ;\n"
			///ra0.16b = int(r3)
			"sig_none ; rx0.16b = ftoi.always(r3, r3) ; nop = nop(r0, r0) ;\n"
			///set up VPM write for subsequent writes
			///0x00001a00: 0000 0000 0000 0000 0001 1010 0000 0000
			///addr: 0
			///size: 32bit
			///horizontal
			///stride = 1
			"sig_load_imm ; vw_setup = load32.always.ws(0x00001a00) ; nop = load32.always() ;\n"
			///shaded vertex format for PSE
			/// Ys and Xs
			///vpm = ra0
			"sig_none ; vpm = or.always(a, a, ra0, nop) ; nop = nop(r0, r0);\n"
			/// Zs
			///uni = 0.5
			///vpm = uni
			"sig_none ; vpm = or.always(a, a, uni, nop) ; nop = nop(r0, r0);\n"
			/// 1.0 / Wc
			///vpm = rb0 (1)
			"sig_none ; vpm = or.always(b, b, nop, rb0) ; nop = nop(r0, r0);\n"
			///END
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
				"\0";

	char cs_asm_code[] =
			///uni = 1.0
			///r3 = 2.0 - uni
			"sig_small_imm ; r3 = fsub.always(b, a, uni, 0x40000000) ; nop = nop(r0, r0);\n"
			"sig_load_imm ; vr_setup = load32.always(0x00201a00) ; nop = load32.always() ;\n"
			///r2 = vpm
			"sig_none ; r2 = or.always(a, a, vpm_read, nop) ; nop = nop(r0, r0);\n"
			"sig_load_imm ; vw_setup = load32.always.ws(0x00001a00) ; nop = load32.always() ;\n"
			///shaded coordinates format for PTB
			/// write Xc
			///r1 = vpm, vpm = r2
			"sig_none ; r1 = or.always(a, a, vpm_read, nop) ; vpm = v8min.always(r2, r2);\n"
			/// write Yc
			///uni = viewportXscale
			///vpm = r1, r2 = r2 * uni
			"sig_none ; vpm = or.always(r1, r1, uni, nop) ; r2 = fmul.always(r2, a);\n"
			///uni = viewportYscale
			///r1 = r1 * uni
			"sig_none ; nop = nop(r0, r0, uni, nop) ; r1 = fmul.always(r1, a);\n"
			///r0 = r2 * r3
			"sig_none ; nop = nop(r0, r0) ; r0 = fmul.always(r2, r3);\n"
			///ra0.16a = r0, r1 = r1 * r3
			"sig_none ; rx0.16a = ftoi.always(r0, r0) ; r1 = fmul.always(r1, r3) ;\n"
			///ra0.16b = r1
			"sig_none ; rx0.16b = ftoi.always(r1, r1) ; nop = nop(r0, r0) ;\n"
			///write Zc
			///vpm = 0
			"sig_small_imm ; vpm = or.always(b, b, nop, 0) ; nop = nop(r0, r0) ;\n"
			///write Wc
			///vpm = 1.0
			"sig_small_imm ; vpm = or.always(b, b, nop, 0x3f800000) ; nop = nop(r0, r0) ;\n"
			///write Ys and Xs
			///vpm = ra0
			"sig_none ; vpm = or.always(a, a, ra0, nop) ; nop = nop(r0, r0) ;\n"
			///write Zs
			///uni = 0.5
			///vpm = uni
			"sig_none ; vpm = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;\n"
			///write 1/Wc
			///vpm = r3
			"sig_none ; vpm = or.always(r3, r3) ; nop = nop(r0, r0) ;\n"
			///END
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;\n"
				"\0";

	//sample texture
	char fs_asm_code[] =
			"sig_none ; r0 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //clear color value
			"sig_none ; r1 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //stencil setup
			"sig_none ; r2 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //depth clear value
			"sig_none ; tlb_stencil_setup = or.always(r1, r1) ; nop = nop(r0, r0) ;"
			"sig_none ; tlb_z = or.always(r2, r2) ; nop = nop(r0, r0) ;"
			"sig_none ; tlb_color_all = or.always(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_unlock_score ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
				"\0";

	char fs_asm_code_no_color[] =
			"sig_none ; r0 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //clear color value
			"sig_none ; r1 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //stencil setup
			"sig_none ; r2 = or.always(a, a, uni, nop) ; nop = nop(r0, r0) ;" //depth clear value
			"sig_none ; tlb_stencil_setup = or.always(r1, r1) ; nop = nop(r0, r0) ;"
			"sig_none ; tlb_z = or.always(r2, r2) ; nop = nop(r0, r0) ;"
			///"sig_none ; tlb_color_all = or.always(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_unlock_score ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
				"\0";

	VkRpiAssemblyMappingEXT vertexMappings[] = {
		//vertex shader uniforms
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			0, //resource offset
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			4, //resource offset
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			8, //resource offset
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			12, //resource offset
		},
	};

	VkRpiAssemblyMappingEXT fragmentMappings[] = {
		//fragment shader uniforms
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			0, //resource offset
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			4, //resource offset
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			8, //resource offset
		},

	};

	uint32_t spirv[6];

	uint64_t* asm_ptrs[4] = {};
	uint32_t asm_sizes[4] = {};

	VkRpiAssemblyMappingEXT* asm_mappings[4] = {};
	uint32_t asm_mappings_sizes[4] = {};

	VkRpiShaderModuleAssemblyCreateInfoEXT shaderModuleCreateInfo = {0};
	shaderModuleCreateInfo.instructions = asm_ptrs;
	shaderModuleCreateInfo.numInstructions = asm_sizes;
	shaderModuleCreateInfo.mappings = asm_mappings;
	shaderModuleCreateInfo.numMappings = asm_mappings_sizes;

	asm_mappings[VK_RPI_ASSEMBLY_TYPE_VERTEX] = vertexMappings;
	asm_mappings_sizes[VK_RPI_ASSEMBLY_TYPE_VERTEX] = sizeof(vertexMappings) / sizeof(VkRpiAssemblyMappingEXT);
	asm_mappings[VK_RPI_ASSEMBLY_TYPE_FRAGMENT] = fragmentMappings;
	asm_mappings_sizes[VK_RPI_ASSEMBLY_TYPE_FRAGMENT] = sizeof(fragmentMappings) / sizeof(VkRpiAssemblyMappingEXT);

	//TODO use allocator

	{ //assemble cs code
		asm_sizes[0] = get_num_instructions(cs_asm_code);
		uint32_t size = sizeof(uint64_t)*asm_sizes[0];
		//TODO this alloc feels kinda useless
		asm_ptrs[0] = (uint64_t*)malloc(size);
		assemble_qpu_asm(cs_asm_code, asm_ptrs[0]);
		assert(asm_ptrs[0]);
	}

	{ //assemble vs code
		asm_sizes[1] = get_num_instructions(vs_asm_code);
		uint32_t size = sizeof(uint64_t)*asm_sizes[1];
		//TODO this alloc feels kinda useless
		asm_ptrs[1] = (uint64_t*)malloc(size);
		assemble_qpu_asm(vs_asm_code, asm_ptrs[1]);
		assert(asm_ptrs[1]);
	}

	{ //assemble fs code
		asm_sizes[2] = get_num_instructions(fs_asm_code);
		uint32_t size = sizeof(uint64_t)*asm_sizes[2];
		//TODO this alloc feels kinda useless
		asm_ptrs[2] = (uint64_t*)malloc(size);
		assemble_qpu_asm(fs_asm_code, asm_ptrs[2]);
		assert(asm_ptrs[2]);
	}

	spirv[0] = 0x07230203;
	spirv[1] = 0x00010000;
	spirv[2] = 0x14E45250;
	spirv[3] = 1;
	spirv[4] = (uint32_t)&shaderModuleCreateInfo;
	//words start here
	spirv[5] = 1 << 16;

	VkShaderModuleCreateInfo smci = {0};
	smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	smci.codeSize = sizeof(uint32_t)*6;
	smci.pCode = spirv;
	RPIFUNC(vkCreateShaderModule)(device, &smci, 0, blitShaderModule);
	assert(*blitShaderModule);

	{ //assemble fs code
		asm_sizes[2] = get_num_instructions(fs_asm_code_no_color);
		uint32_t size = sizeof(uint64_t)*asm_sizes[2];
		//TODO this alloc feels kinda useless, we just copy the data anyway to kernel space
		//why not map kernel space mem to user space instead?
		free(asm_ptrs[2]);
		asm_ptrs[2] = (uint64_t*)malloc(size);
		assemble_qpu_asm(fs_asm_code_no_color, asm_ptrs[2]);
		assert(asm_ptrs[2]);
	}

	RPIFUNC(vkCreateShaderModule)(device, &smci, 0, blitShaderModuleNoColor);
	assert(*blitShaderModuleNoColor);

	for(uint32_t c = 0; c < 4; ++c)
	{
		free(asm_ptrs[c]);
	}
}

void createClearPipeline(VkDevice device, VkPipelineDepthStencilStateCreateInfo* dsState, VkShaderModule blitShaderModule, VkDescriptorSetLayout blitDsl, VkPipelineLayout* blitPipelineLayout, VkRenderPass offscreenRenderPass, VkPipeline* blitPipeline)
{
	VkVertexInputBindingDescription vertexInputBindingDescription =
	{
		0,
		sizeof(float) * 2 * 2,
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	VkVertexInputAttributeDescription vertexInputAttributeDescription[2];

	{
		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].offset = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

	VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {0};
	pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {0};
	rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rastCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rastCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {0};
	pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState blendAttachState = {0};
	blendAttachState.colorWriteMask = 0xf;
	blendAttachState.blendEnable = false;

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {0};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = &blendAttachState;

	//create blit pipeline
	VkPushConstantRange pushConstantRanges[2];
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = 4 * 4; //n * 32bits
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pushConstantRanges[1].offset = 0;
	pushConstantRanges[1].size = 3 * 4; //n * 32bits
	pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};

	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = blitShaderModule;
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = blitShaderModule;
	shaderStageCreateInfo[1].pName = "main";

	VkPipelineLayoutCreateInfo pipelineLayoutCI = {0};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &blitDsl;
	pipelineLayoutCI.pushConstantRangeCount = 2;
	pipelineLayoutCI.pPushConstantRanges = &pushConstantRanges[0];
	RPIFUNC(vkCreatePipelineLayout)(device, &pipelineLayoutCI, 0, blitPipelineLayout);

	VkDynamicState dynState = VK_DYNAMIC_STATE_VIEWPORT;

	VkPipelineDynamicStateCreateInfo pdsci = {0};
	pdsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pdsci.dynamicStateCount = 1;
	pdsci.pDynamicStates = &dynState;

	VkPipelineViewportStateCreateInfo pvsci = {0};
	pvsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pvsci.viewportCount = 0;
	pvsci.scissorCount = 0;

	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = &shaderStageCreateInfo[0];
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
	pipelineInfo.pViewportState = &pvsci;
	pipelineInfo.pDynamicState = &pdsci;
	pipelineInfo.pRasterizationState = &rastCreateInfo;
	pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
	pipelineInfo.pColorBlendState = &blendCreateInfo;
	pipelineInfo.renderPass = offscreenRenderPass;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.pDepthStencilState = dsState;
	pipelineInfo.layout = *blitPipelineLayout;

	RPIFUNC(vkCreateGraphicsPipelines)(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, blitPipeline);
}

void createClearDescriptorSetLayouts(VkDevice device, VkDescriptorSetLayout* bufferDsl)
{
	assert(device);
	assert(bufferDsl);

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI = {0};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = 0;
	descriptorLayoutCI.pBindings = 0;

	RPIFUNC(vkCreateDescriptorSetLayout)(device, &descriptorLayoutCI, 0, bufferDsl);
}

void setupClearEmulationResources(VkDevice device)
{
	//create resources that won't change
	_device* dev = device;

	createClearShaderModule(device, &dev->emulClearShaderModule, &dev->emulClearNoColorShaderModule);
	createClearDescriptorSetLayouts(device, &dev->emulClearDsl);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetViewport
 */
void RPIFUNC(vkCmdSetViewport)(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	PROFILESTART(RPIFUNC(vkCmdSetViewport));

	assert(commandBuffer);
	assert(firstViewport == 0);
	assert(viewportCount == 1);
	assert(pViewports);

	//only 1 viewport is supported

	_commandBuffer* cb = commandBuffer;
	cb->viewport = pViewports[0];

	cb->viewportDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetViewport));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetScissor
 */
void RPIFUNC(vkCmdSetScissor)(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	PROFILESTART(RPIFUNC(vkCmdSetScissor));

	assert(commandBuffer);
	assert(firstScissor == 0);
	assert(scissorCount == 1);
	assert(pScissors);

	//only 1 scissor supported

	_commandBuffer* cb = commandBuffer;
	cb->scissor = pScissors[0];

	cb->scissorDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetScissor));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindVertexBuffers
 */
void RPIFUNC(vkCmdBindVertexBuffers)(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	PROFILESTART(RPIFUNC(vkCmdBindVertexBuffers));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	for(uint32_t c = 0; c < bindingCount; ++c)
	{
		cb->vertexBuffers[firstBinding + c] = pBuffers[c];
		cb->vertexBufferOffsets[firstBinding + c] = pOffsets[c];
	}

	cb->vertexBufferDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdBindVertexBuffers));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearColorImage
 * Color and depth/stencil images can be cleared outside a render pass instance using vkCmdClearColorImage or vkCmdClearDepthStencilImage, respectively.
 * These commands are only allowed outside of a render pass instance.
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearColorImage)(
		VkCommandBuffer                             commandBuffer,
		VkImage                                     image,
		VkImageLayout                               imageLayout,
		const VkClearColorValue*                    pColor,
		uint32_t                                    rangeCount,
		const VkImageSubresourceRange*              pRanges)
{
	PROFILESTART(RPIFUNC(vkCmdClearColorImage));

	assert(commandBuffer);
	assert(image);
	assert(pColor);

	//TODO this should only flag an image for clearing. This can only be called outside a renderpass
	//actual clearing would only happen:
	// -if image is rendered to (insert clear before first draw call)
	// -if the image is bound for sampling (submit a CL with a clear)
	// -if a command buffer is submitted without any rendering (insert clear)
	// -etc.
	//we shouldn't clear an image if noone uses it

	//TODO ranges support

	assert(commandBuffer->state	 == CMDBUF_STATE_RECORDING);
	assert(_queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT || _queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT);

	_image* i = image;

	assert(i->usageBits & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	{ //Simplest case: just submit a job to clear the image
		clFit(&commandBuffer->binCl, sizeof(CLMarker));
		clInsertNewCLMarker(&commandBuffer->binCl, &commandBuffer->handlesCl, &commandBuffer->shaderRecCl, commandBuffer->shaderRecCount, &commandBuffer->uniformsCl);

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->writeImage = i;

		//insert reloc for render target
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->handlesBufOffset + commandBuffer->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->handlesSize, i->boundMem->bo);

		clFit(&commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
		clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
											 0, //double buffer in non ms mode
											 0, //tile allocation block size
											 0, //tile allocation initial block size
											 0, //auto initialize tile state data array
											 getFormatBpp(i->format) == 64, //64 bit color mode
											 i->samples > 1, //msaa
											 i->width, i->height,
											 0, //tile state data array address
											 0, //tile allocation memory size
											 0); //tile allocation memory address

		//START_TILE_BINNING resets the statechange counters in the hardware,
		//which are what is used when a primitive is binned to a tile to
		//figure out what new state packets need to be written to that tile's
		//command list.
		clFit(&commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
		clInsertStartTileBinning(&commandBuffer->binCl);

		//Increment the semaphore indicating that binning is done and
		//unblocking the render thread.  Note that this doesn't act
		//until the FLUSH completes.
		//The FLUSH caps all of our bin lists with a
		//VC4_PACKET_RETURN.
		clFit(&commandBuffer->binCl, V3D21_INCREMENT_SEMAPHORE_length);
		clInsertIncrementSemaphore(&commandBuffer->binCl);
		clFit(&commandBuffer->binCl, V3D21_FLUSH_length);
		clInsertFlush(&commandBuffer->binCl);

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->clearColor[0] = ((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->clearColor[1] = packVec4IntoABGR8(pColor->float32);
		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->width = i->width;
		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->height = i->height;
	}

	PROFILEEND(RPIFUNC(vkCmdClearColorImage));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearDepthStencilImage
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearDepthStencilImage)(
	VkCommandBuffer                             commandBuffer,
	VkImage                                     image,
	VkImageLayout                               imageLayout,
	const VkClearDepthStencilValue*             pDepthStencil,
	uint32_t                                    rangeCount,
	const VkImageSubresourceRange*              pRanges)
{
	PROFILESTART(RPIFUNC(vkCmdClearDepthStencilImage));

	assert(commandBuffer);
	assert(image);
	assert(pDepthStencil);

	//TODO this should only flag an image for clearing. This can only be called outside a renderpass
	//actual clearing would only happen:
	// -if image is rendered to (insert clear before first draw call)
	// -if the image is bound for sampling (submit a CL with a clear)
	// -if a command buffer is submitted without any rendering (insert clear)
	// -etc.
	//we shouldn't clear an image if noone uses it

	//TODO ranges support

	assert(commandBuffer->state	 == CMDBUF_STATE_RECORDING);
	assert(_queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT || _queueFamilyProperties[commandBuffer->cp->queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT);

	_image* i = image;

	assert(i->usageBits & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	{ //Simplest case: just submit a job to clear the image
		clFit(&commandBuffer->binCl, sizeof(CLMarker));
		clInsertNewCLMarker(&commandBuffer->binCl, &commandBuffer->handlesCl, &commandBuffer->shaderRecCl, commandBuffer->shaderRecCount, &commandBuffer->uniformsCl);

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->writeDepthStencilImage = i;

		//insert reloc for render target
		clFit(&commandBuffer->handlesCl, 4);
		clGetHandleIndex(&commandBuffer->handlesCl, ((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->handlesBufOffset + commandBuffer->handlesCl.offset, ((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->handlesSize, i->boundMem->bo);

		clFit(&commandBuffer->binCl, V3D21_TILE_BINNING_MODE_CONFIGURATION_length);
		clInsertTileBinningModeConfiguration(&commandBuffer->binCl,
											 0, //double buffer in non ms mode
											 0, //tile allocation block size
											 0, //tile allocation initial block size
											 0, //auto initialize tile state data array
											 0, //64 bit color mode
											 i->samples > 1, //msaa
											 i->width, i->height,
											 0, //tile state data array address
											 0, //tile allocation memory size
											 0); //tile allocation memory address

		//START_TILE_BINNING resets the statechange counters in the hardware,
		//which are what is used when a primitive is binned to a tile to
		//figure out what new state packets need to be written to that tile's
		//command list.
		clFit(&commandBuffer->binCl, V3D21_START_TILE_BINNING_length);
		clInsertStartTileBinning(&commandBuffer->binCl);

		//Increment the semaphore indicating that binning is done and
		//unblocking the render thread.  Note that this doesn't act
		//until the FLUSH completes.
		//The FLUSH caps all of our bin lists with a
		//VC4_PACKET_RETURN.
		clFit(&commandBuffer->binCl, V3D21_INCREMENT_SEMAPHORE_length);
		clInsertIncrementSemaphore(&commandBuffer->binCl);
		clFit(&commandBuffer->binCl, V3D21_FLUSH_length);
		clInsertFlush(&commandBuffer->binCl);

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->clearDepth = (uint32_t)(pDepthStencil->depth * 0xffffff) & 0xffffff;
		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->clearStencil = pDepthStencil->stencil & 0xff;
		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;

		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->width = i->width;
		((CLMarker*)getCPAptrFromOffset(commandBuffer->binCl.CPA, commandBuffer->binCl.currMarkerOffset))->height = i->height;
	}

	PROFILEEND(RPIFUNC(vkCmdClearDepthStencilImage));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdClearAttachments
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdClearAttachments)(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    attachmentCount,
	const VkClearAttachment*                    pAttachments,
	uint32_t                                    rectCount,
	const VkClearRect*                          pRects)
{
	PROFILESTART(RPIFUNC(vkCmdClearAttachments));

	assert(commandBuffer);
	assert(pAttachments);
	assert(pRects);

	_commandBuffer* cmdBuf = commandBuffer;
	_device* device = cmdBuf->dev;

	if(!cmdBuf->currRenderPass)
	{
		//no active render pass
		PROFILEEND(RPIFUNC(vkCmdClearAttachments));
		return;
	}

	_pipeline* oldPipeline;
	uint32_t oldVertexBufferOffsets[8];
	_buffer* oldVertexBuffers[8];
	char oldPushConstantBufferVertex[256];
	char oldPushConstantBufferPixel[256];
	VkViewport oldViewport;

	//save the state that we'll modify
	oldViewport = cmdBuf->viewport;
	oldPipeline = cmdBuf->graphicsPipeline;
	memcpy(oldVertexBufferOffsets, cmdBuf->vertexBufferOffsets, sizeof(oldVertexBufferOffsets));
	memcpy(oldVertexBuffers, cmdBuf->vertexBuffers, sizeof(oldVertexBuffers));
	memcpy(oldPushConstantBufferVertex, cmdBuf->pushConstantBufferVertex, sizeof(uint32_t) * 10);
	memcpy(oldPushConstantBufferPixel, cmdBuf->pushConstantBufferPixel, sizeof(uint32_t) * 10);

	for(uint32_t c = 0; c < attachmentCount; ++c)
	{
		uint32_t clearColor = 0, clearDepth = 0, clearStencil = 0;

		if(pAttachments[c].aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
		{
				clearColor = 1;
		}

		if(pAttachments[c].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
		{
				clearDepth = 1;
		}

		if(pAttachments[c].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
		{
				clearStencil = 1;
		}

		VkPipeline blitPipeline;
		VkPipelineLayout blitPipelineLayout;

		VkPipelineDepthStencilStateCreateInfo dsci = {0};
		dsci.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		dsci.depthTestEnable = 1;
		dsci.depthWriteEnable = clearDepth;
		dsci.minDepthBounds = 0.0f;
		dsci.maxDepthBounds = 1.0f;
		dsci.stencilTestEnable = clearStencil;
		dsci.front.compareMask = 0xff;
		dsci.front.writeMask = 0xff;
		dsci.front.reference = pAttachments[c].clearValue.depthStencil.stencil;
		dsci.front.depthFailOp = VK_STENCIL_OP_REPLACE;
		dsci.front.failOp = VK_STENCIL_OP_REPLACE;
		dsci.front.passOp = VK_STENCIL_OP_REPLACE;
		dsci.back = dsci.front;

		//TODO cache pipeline, don't create it each occasion
		createClearPipeline(device, &dsci, clearColor ? device->emulClearShaderModule : device->emulClearNoColorShaderModule, device->emulClearDsl, &blitPipelineLayout, cmdBuf->currRenderPass, &blitPipeline);

		RPIFUNC(vkCmdBindPipeline)(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blitPipeline);

		VkDeviceSize offsets = 0;
		RPIFUNC(vkCmdBindVertexBuffers)(commandBuffer, 0, 1, &device->emulFsqVertexBuffer, &offsets );

		uint32_t clearColorValue = 0, stencilSetup = 0, depthClearValue = 0;

		clearColorValue = packVec4IntoABGR8(&pAttachments[c].clearValue.color.float32[0]);
		depthClearValue = (uint32_t)(pAttachments[c].clearValue.depthStencil.depth * 0xffffffu) & 0xffffffu;
		uint32_t numValues = 1;
		encodeStencilValue(&stencilSetup, &numValues, dsci.front, dsci.back, clearStencil);


		uint32_t fragConstants[3];
		fragConstants[0] = clearColorValue;
		fragConstants[1] = stencilSetup;
		fragConstants[2] = depthClearValue;

		RPIFUNC(vkCmdPushConstants)(commandBuffer, blitPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(fragConstants), &fragConstants);

		for(uint32_t d = 0; d < rectCount; ++d)
		{
			VkViewport vp = {0};
			vp.x = pRects[d].rect.offset.x;
			vp.y = pRects[d].rect.offset.y;
			vp.width = pRects[d].rect.extent.width;
			vp.height = pRects[d].rect.extent.height;
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;

			RPIFUNC(vkCmdSetViewport)(commandBuffer, 0, 1, &vp);

			float Wcoeff = 1.0f; //1.0f / Wc = 2.0 - Wcoeff
			float viewportScaleX = (float)(vp.width) * 0.5f * 16.0f;
			float viewportScaleY = 1.0f * (float)(vp.height) * 0.5f * 16.0f;
			float Zs = 1.0f;

			uint32_t vertConstants[4];
			vertConstants[0] = *(uint32_t*)&Wcoeff;
			vertConstants[1] = *(uint32_t*)&viewportScaleX;
			vertConstants[2] = *(uint32_t*)&viewportScaleY;
			vertConstants[3] = *(uint32_t*)&Zs;

			RPIFUNC(vkCmdPushConstants)(commandBuffer, blitPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertConstants), &vertConstants);

			RPIFUNC(vkCmdDraw)(commandBuffer, 6, 1, 0, 0);
		}

		//free up resources
		RPIFUNC(vkDestroyPipelineLayout)(device, blitPipelineLayout, 0);
		RPIFUNC(vkDestroyPipeline)(device, blitPipeline, 0);
	}

	//restore state
	cmdBuf->viewport = oldViewport;
	cmdBuf->graphicsPipeline = oldPipeline;
	memcpy(cmdBuf->vertexBufferOffsets, oldVertexBufferOffsets, sizeof(oldVertexBufferOffsets));
	memcpy(cmdBuf->vertexBuffers, oldVertexBuffers, sizeof(oldVertexBuffers));
	memcpy(cmdBuf->pushConstantBufferVertex, oldPushConstantBufferVertex, sizeof(uint32_t) * 10);
	memcpy(cmdBuf->pushConstantBufferPixel, oldPushConstantBufferPixel, sizeof(uint32_t) * 10);

	PROFILEEND(RPIFUNC(vkCmdClearAttachments));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdFillBuffer
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdFillBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                size,
	uint32_t                                    data)
{
	//TODO need kernel linear format support
	UNSUPPORTED(vkCmdFillBuffer);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdUpdateBuffer
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdUpdateBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    dstBuffer,
	VkDeviceSize                                dstOffset,
	VkDeviceSize                                dataSize,
	const void*                                 pData)
{
	//TODO need kernel linear format support
	UNSUPPORTED(vkCmdFillBuffer);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBindIndexBuffer
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdBindIndexBuffer)(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	VkIndexType                                 indexType)
{
	PROFILESTART(RPIFUNC(vkCmdBindIndexBuffer));

	assert(commandBuffer);

	if(indexType == VK_INDEX_TYPE_UINT32)
	{
		UNSUPPORTED(VK_INDEX_TYPE_UINT32);
	}

	_commandBuffer* cb = commandBuffer;

	cb->indexBuffer = buffer;
	cb->indexBufferOffset = offset;

	cb->indexBufferDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdBindIndexBuffer));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetLineWidth
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetLineWidth)(
	VkCommandBuffer                             commandBuffer,
	float                                       lineWidth)
{
	PROFILESTART(RPIFUNC(vkCmdSetLineWidth));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->lineWidth = lineWidth;

	cb->lineWidthDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetLineWidth));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetDepthBias
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDepthBias)(
	VkCommandBuffer                             commandBuffer,
	float                                       depthBiasConstantFactor,
	float                                       depthBiasClamp,
	float                                       depthBiasSlopeFactor)
{
	PROFILESTART(RPIFUNC(vkCmdSetDepthBias));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->depthBiasConstantFactor = depthBiasConstantFactor;
	cb->depthBiasClamp = depthBiasClamp;
	cb->depthBiasSlopeFactor = depthBiasSlopeFactor;

	cb->depthBiasDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetDepthBias));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetBlendConstants
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetBlendConstants)(
	VkCommandBuffer                             commandBuffer,
	const float                                 blendConstants[4])
{
	PROFILESTART(RPIFUNC(vkCmdSetBlendConstants));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	memcpy(cb->blendConstants, blendConstants, 4 * sizeof(float));

	cb->blendConstantsDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetBlendConstants));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetDepthBounds
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetDepthBounds)(
	VkCommandBuffer                             commandBuffer,
	float                                       minDepthBounds,
	float                                       maxDepthBounds)
{
	PROFILESTART(RPIFUNC(vkCmdSetDepthBounds));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;
	cb->minDepthBounds = minDepthBounds;
	cb->maxDepthBounds = maxDepthBounds;

	cb->depthBoundsDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetDepthBounds));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilCompareMask
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilCompareMask)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    compareMask)
{
	PROFILESTART(RPIFUNC(vkCmdSetStencilCompareMask));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilCompareMask[0] = compareMask;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilCompareMask[1] = compareMask;
	}

	cb->stencilCompareMaskDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetStencilCompareMask));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilWriteMask
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilWriteMask)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    writeMask)
{
	PROFILESTART(RPIFUNC(vkCmdSetStencilWriteMask));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilWriteMask[0] = writeMask;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilWriteMask[1] = writeMask;
	}

	cb->stencilWriteMaskDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetStencilWriteMask));
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdSetStencilReference
 */
VKAPI_ATTR void VKAPI_CALL RPIFUNC(vkCmdSetStencilReference)(
	VkCommandBuffer                             commandBuffer,
	VkStencilFaceFlags                          faceMask,
	uint32_t                                    reference)
{
	PROFILESTART(RPIFUNC(vkCmdSetStencilReference));

	assert(commandBuffer);

	_commandBuffer* cb = commandBuffer;

	if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
	{
		cb->stencilReference[0] = reference;
	}

	if(faceMask & VK_STENCIL_FACE_BACK_BIT)
	{
		cb->stencilReference[1] = reference;
	}

	cb->stencilReferenceDirty = 1;

	PROFILEEND(RPIFUNC(vkCmdSetStencilReference));
}
