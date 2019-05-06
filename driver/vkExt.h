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

typedef enum VkRpiAssemblyTypeEXT {
	VK_RPI_ASSEMBLY_TYPE_COORDINATE = 0,
	VK_RPI_ASSEMBLY_TYPE_VERTEX = 1,
	VK_RPI_ASSEMBLY_TYPE_FRAGMENT = 2,
	VK_RPI_ASSEMBLY_TYPE_COMPUTE = 3,
	VK_RPI_ASSEMBLY_TYPE_MAX,
} VkRpiAssemblyTypeKHR;

typedef struct VkRpiSurfaceCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	VkRpiSurfaceCreateFlagsEXT    flags; //reserved
	//maybe include some other stuff dunno
} VkRpiSurfaceCreateInfoEXT;

typedef struct VkRpiShaderModuleAssemblyCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	char**						  asmStrings;
	uint32_t*					  descriptorBindings;
	uint32_t*					  descriptorSets;
	VkDescriptorType*			  descriptorTypes;
	uint32_t*					  descriptorCounts;
	uint32_t*					  descriptorArrayElems;
	uint32_t*					  numDescriptorBindings;
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
