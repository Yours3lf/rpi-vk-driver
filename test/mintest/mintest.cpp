#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

int main() {

	VkInstance instance;
	VkSurfaceKHR windowSurface;
	VkPhysicalDevice physicalDevice;

	const char* enabledExtensions[] = {
		"VK_KHR_surface",
		"VK_KHR_display",
	};


	VkInstanceCreateInfo createInfo = {};
	createInfo.enabledExtensionCount = sizeof(enabledExtensions) / sizeof(const char*);
	createInfo.ppEnabledExtensionNames = enabledExtensions;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkCreateInstance(&createInfo, nullptr, &instance);

	uint32_t deviceCount = 1;
	vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);

	uint32_t displayCount;
	vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, 0);
	VkDisplayPropertiesKHR* displayProperties = (VkDisplayPropertiesKHR*)malloc(sizeof(VkDisplayPropertiesKHR)*displayCount);
	vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, displayProperties);

	uint32_t modeCount;
	vkGetDisplayModePropertiesKHR(physicalDevice, displayProperties[0].display, &modeCount, 0);
	VkDisplayModePropertiesKHR* displayModeProperties = (VkDisplayModePropertiesKHR*)malloc(sizeof(VkDisplayModePropertiesKHR)*modeCount);
	vkGetDisplayModePropertiesKHR(physicalDevice, displayProperties[0].display, &modeCount, displayModeProperties);

//	printf("\nEnumerated modes\n");
//	for(uint32_t c = 0; c < modeCount; ++c)
//	{
//		printf("Mode refresh rate %i\n", displayModeProperties[c].parameters.refreshRate);
//		printf("Mode width %i\n", displayModeProperties[c].parameters.visibleRegion.width);
//		printf("Mode height %i\n\n", displayModeProperties[c].parameters.visibleRegion.height);
//	}


	VkDisplaySurfaceCreateInfoKHR dsci = {};
	dsci.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	vkCreateDisplayPlaneSurfaceKHR(instance, &dsci, 0, &windowSurface);

	return 0;
}
