#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef VkResult (*PFN_vkCreateRpiSurfaceEXT)(
			VkPhysicalDevice                            physicalDevice);
typedef VkResult (*PFN_vkCreateShaderModuleFromRpiAssemblyEXT)(
			VkPhysicalDevice                            physicalDevice);

// Sooo we're not really getting the REAL VkPhysicalDevice from the Loader
// But rather a Trampoline object that points to a Terminator that finally points to
// The real object
// Therefore if we would like to pass on information in our VkPhysicalDevice object
// We need to walk this chain...
typedef struct VkRpiPhysicalDevice
{
	uintptr_t loaderData;
	uintptr_t customData;
} VkRpiPhysicalDevice;

typedef struct LoaderTerminator
{
	uintptr_t a;
	uintptr_t b;
	uint8_t c;
	VkRpiPhysicalDevice* physicalDevice;
} LoaderTerminator;

typedef struct LoaderTrampoline
{
	uintptr_t a;
	uintptr_t b;
	LoaderTerminator* loaderTerminator;
} LoaderTrampoline;

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

typedef enum VkRpiSurfaceCreateFlagsEXT {
	//reserved
	VK_RPI_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkRpiSurfaceCreateFlagsEXT;


typedef enum VkRpiAssemblyMappingTypeEXT {
	VK_RPI_ASSEMBLY_MAPPING_TYPE_DESCRIPTOR = 0,
	VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT = 1,
	VK_RPI_ASSEMBLY_MAPPING_TYPE_MAX
} VkRpiAssemblyMappingTypeEXT;

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

//we need something like the other platforms to create surfaces on the RPI
//so I created this little "extension"
//full spec in this file ;)

typedef struct VkRpiSurfaceCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	VkRpiSurfaceCreateFlagsEXT    flags; //reserved
	const VkAllocationCallbacks*  pAllocator;
	VkSurfaceKHR*				  pSurface;
} VkRpiSurfaceCreateInfoEXT;

typedef struct VkRpiShaderModuleAssemblyCreateInfoEXT {
	VkStructureType               sType;
	const void*                   pNext;
	char**						  asmStrings;
	VkRpiAssemblyMappingEXT*	  mappings;
	uint32_t					  numMappings;
	const VkAllocationCallbacks*  pAllocator;
	VkShaderModule*				  pShaderModule;
} VkRpiShaderModuleAssemblyCreateInfoEXT;

#ifdef __cplusplus
}
#endif

