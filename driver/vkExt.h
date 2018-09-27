#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//we need something like the other platforms to create surfaces on the RPI
//so I created this little "extension"
//full spec in this file ;)

typedef enum VkRpiSurfaceCreateFlagsKHR {
	//reserved
	VK_RPI_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkRpiSurfaceCreateFlagsKHR;

typedef enum VkRpiAssemblyTypeKHR {
	VK_RPI_ASSEMBLY_TYPE_COORDINATE = 0,
	VK_RPI_ASSEMBLY_TYPE_VERTEX = 1,
	VK_RPI_ASSEMBLY_TYPE_FRAGMENT = 2,
	VK_RPI_ASSEMBLY_TYPE_COMPUTE = 3,
	VK_RPI_ASSEMBLY_TYPE_MAX,
} VkRpiAssemblyTypeKHR;

typedef struct VkRpiSurfaceCreateInfoKHR {
	VkStructureType               sType;
	const void*                   pNext;
	VkRpiSurfaceCreateFlagsKHR    flags; //reserved
	//maybe include some other stuff dunno
} VkRpiSurfaceCreateInfoKHR;

typedef struct VkRpiShaderModuleAssemblyCreateInfoKHR {
	VkStructureType               sType;
	const void*                   pNext;
	char*						  byteStreamArray[VK_RPI_ASSEMBLY_TYPE_MAX];
	uint32_t					  numBytesArray[VK_RPI_ASSEMBLY_TYPE_MAX];
} VkRpiShaderModuleAssemblyCreateInfoKHR;

//extension name something like: VK_KHR_rpi_surface
//extension that allows developers to create a surface to render to on Raspbian Stretch Lite
VkResult vkCreateRpiSurfaceKHR(
		VkInstance                                  instance,
		const VkRpiSurfaceCreateInfoKHR*            pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkSurfaceKHR*                               pSurface);

//extension that allows developers to submit QPU assembly directly and thus hand optimise code
VkResult vkCreateShaderModuleFromRpiAssemblyKHR(
		VkDevice									device,
		VkRpiShaderModuleAssemblyCreateInfoKHR*		pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkShaderModule*								pShaderModule
		);


#ifdef __cplusplus
}
#endif
