#include "common.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceExtensionProperties
 * When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation or by implicitly enabled layers are returned. When pLayerName is the name of a layer,
 * the instance extensions provided by that layer are returned.
 * If pProperties is NULL, then the number of extensions properties available is returned in pPropertyCount. Otherwise, pPropertyCount must point to a variable set by the user
 * to the number of elements in the pProperties array, and on return the variable is overwritten with the number of structures actually written to pProperties.
 * If pPropertyCount is less than the number of extension properties available, at most pPropertyCount structures will be written. If pPropertyCount is smaller than the number of extensions available,
 * VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the available properties were returned.
 * Because the list of available layers may change externally between calls to vkEnumerateInstanceExtensionProperties,
 * two calls may retrieve different results if a pLayerName is available in one call but not in another. The extensions supported by a layer may also change between two calls,
 * e.g. if the layer implementation is replaced by a different version between those calls.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(!pLayerName); //TODO layers ignored for now
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numInstanceExtensions;
		return VK_INCOMPLETE;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numInstanceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = instanceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateInstance
 * There is no global state in Vulkan and all per-application state is stored in a VkInstance object. Creating a VkInstance object initializes the Vulkan library
 * vkCreateInstance verifies that the requested layers exist. If not, vkCreateInstance will return VK_ERROR_LAYER_NOT_PRESENT. Next vkCreateInstance verifies that
 * the requested extensions are supported (e.g. in the implementation or in any enabled instance layer) and if any requested extension is not supported,
 * vkCreateInstance must return VK_ERROR_EXTENSION_NOT_PRESENT. After verifying and enabling the instance layers and extensions the VkInstance object is
 * created and returned to the application.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
		const VkInstanceCreateInfo*                 pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkInstance*                                 pInstance)
{
	assert(pInstance);
	assert(pCreateInfo);

	*pInstance = malloc(sizeof(_instance));

	if(!*pInstance)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	(*pInstance)->numEnabledExtensions = 0;

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	//TODO: possibly we need to load layers here
	//and store them in pInstance
	assert(pCreateInfo->enabledLayerCount == 0);

	if(pCreateInfo->enabledExtensionCount)
	{
		assert(pCreateInfo->ppEnabledExtensionNames);
	}

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findInstanceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pInstance)->enabledExtensions[(*pInstance)->numEnabledExtensions] = findres;
			(*pInstance)->numEnabledExtensions++;
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//TODO ignored for now
	//pCreateInfo->pApplicationInfo

	int ret = openIoctl(); assert(!ret);

	(*pInstance)->chipVersion = vc4_get_chip_info(controlFd);
	(*pInstance)->hasTiling = vc4_test_tiling(controlFd);

	(*pInstance)->hasControlFlow = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
	(*pInstance)->hasEtc1 = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_ETC1);
	(*pInstance)->hasThreadedFs = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
	(*pInstance)->hasMadvise = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_MADVISE);

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyInstance
 *
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(
		VkInstance                                  instance,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(instance);

	//TODO: allocator is ignored for now
	assert(pAllocator == 0);

	closeIoctl();

	free(instance);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceVersion
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(
	uint32_t*                                   pApiVersion)
{
	assert(pApiVersion);
	*pApiVersion = VK_MAKE_VERSION(1, 1, 0);
	return VK_SUCCESS;
}
