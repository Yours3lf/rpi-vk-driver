#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//we need something like the other platforms to create surfaces on the RPI
//so I created this little "extension"
//full spec in this file ;)

typedef enum VkRpiSurfaceCreateFlagsEXT {
	//reserved
	VK_RPI_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkRpiSurfaceCreateFlagsEXT;

typedef struct VkRpiSurfaceCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	VkRpiSurfaceCreateFlagsEXT    flags; //reserved
	//maybe include some other stuff dunno
} VkRpiSurfaceCreateInfoEXT;

typedef enum VkRpiAssemblyMappingTypeEXT {
	VK_RPI_ASSEMBLY_MAPPING_TYPE_DESCRIPTOR = 0,
	VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT = 1,
	VK_RPI_ASSEMBLY_MAPPING_TYPE_MAX
} VkRpiAssemblyMappingTypeEXT;

/*
 * assembly to vulkan resource mapping
 *
 * map vulkan resources such as
 * -push constants
 * -descriptor set entries
 *	-images
 *  -buffers
 *
 * to assembly uniform reads
 *
 * push constants should be one read
 *
 * buffers and images are handled through the TMU pipeline
 * and therefore carry implicit uniform reads
 * buffers should be one uniform (general memory read)
 * number of uniforms for images are dependent on type (and TMU writes)
 *
 * therefore what we need is a mapping for each assembly uniform read
 * to some vulkan resource
 * and the driver should be able to figure out what to put in the uniform queue
 * based on the mapping
 *
 * vertex and coordinate shader mappings are shared
 *
 */

//defines mapping for a single uniform FIFO read to a Vulkan resource
typedef struct VkRpiAssemblyMappingEXT {
	VkRpiAssemblyMappingTypeEXT	mappingType;
	VkDescriptorType			descriptorType;
	uint32_t					descriptorSet;
	uint32_t					descriptorBinding;
	uint32_t					descriptorArrayElement;
	uint32_t					resourceOffset; //in bytes
	VkShaderStageFlagBits		shaderStage;
} VkRpiAssemblyMappingEXT;

typedef struct VkRpiShaderModuleAssemblyCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	char**						  asmStrings;
	VkRpiAssemblyMappingEXT*	  mappings;
	uint32_t					  numMappings;
} VkRpiShaderModuleAssemblyCreateInfoEXT;

//extension name something like: VK_KHR_rpi_surface
//extension that allows developers to create a surface to render to on Raspbian Stretch Lite
VkResult vkCreateRpiSurfaceEXT(
		VkInstance                                  instance,
		const VkRpiSurfaceCreateInfoEXT*            pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSurfaceKHR*                               pSurface);

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
VkResult vkCreateShaderModuleFromRpiAssemblyEXT(
		VkDevice									device,
		VkRpiShaderModuleAssemblyCreateInfoEXT*		pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkShaderModule*								pShaderModule
		);


#ifdef __cplusplus
}
#endif
