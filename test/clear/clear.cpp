#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include "driver/CustomAssert.h"

#include <vulkan/vulkan.h>

#include "driver/vkExt.h"

//#define GLFW_INCLUDE_VULKAN
//#define VK_USE_PLATFORM_WIN32_KHR
//#include <GLFW/glfw3.h>

//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

//GLFWwindow * window;

VkInstance instance;
VkSurfaceKHR windowSurface;
VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSemaphore imageAvailableSemaphore;
VkSemaphore renderingFinishedSemaphore;
VkSwapchainKHR swapChain;
std::vector<VkImage> swapChainImages;
VkCommandPool commandPool;
std::vector<VkCommandBuffer> presentCommandBuffers;

uint32_t graphicsQueueFamily;
uint32_t presentQueueFamily;

// Note: support swap chain recreation (not only required for resized windows!)
// Note: window resize may not result in Vulkan telling that the swap chain should be recreated, should be handled explicitly!
void run();
void setupVulkan();
void mainLoop();
void cleanup();
void createInstance();
void createWindowSurface();
void findPhysicalDevice();
void checkSwapChainSupport();
void findQueueFamilies();
void createLogicalDevice();
void createSemaphores();
void createSwapChain();
void createCommandQueues();
void draw();
VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> presentModes);

void run() {
	// Note: dynamically loading loader may be a better idea to fail gracefully when Vulkan is not supported

	// Create window for Vulkan
	//glfwInit();

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//window = glfwCreateWindow(640, 480, "The 630 line cornflower blue window", nullptr, nullptr);

	// Use Vulkan
	setupVulkan();

	mainLoop();

	cleanup();
}

void setupVulkan() {
	createInstance();
	createWindowSurface();
	findPhysicalDevice();
	checkSwapChainSupport();
	findQueueFamilies();
	createLogicalDevice();
	createSemaphores();
	createSwapChain();
	createCommandQueues();
}

void mainLoop() {
	//while (!glfwWindowShouldClose(window)) {
	for(int c = 0; c < 10; ++c){
	//for(int c = 0; c < 1; ++c){
		draw();

		//	glfwPollEvents();
	}
}

void cleanup() {
	vkDeviceWaitIdle(device);

	// Note: this is done implicitly when the command pool is freed, but nice to know about
	vkFreeCommandBuffers(device, commandPool, presentCommandBuffers.size(), presentCommandBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderingFinishedSemaphore, nullptr);

	// Note: implicitly destroys images (in fact, we're not allowed to do that explicitly)
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, windowSurface, nullptr);

	vkDestroyInstance(instance, nullptr);
}

void createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanClear";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "ClearScreenEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Get instance extensions required by GLFW to draw to window
	//unsigned int glfwExtensionCount;
	//const char** glfwExtensions;
	//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Check for extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) {
		std::cerr << "no extensions supported!" << std::endl;
		assert(0);
	}

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::cout << "supported extensions:" << std::endl;

	for (const auto& extension : availableExtensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	//createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.enabledExtensionCount = 0;
	//createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.ppEnabledExtensionNames = 0;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = 0;

	// Initialize Vulkan instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		std::cerr << "failed to create instance!" << std::endl;
		assert(0);
	}
	else {
		std::cout << "created vulkan instance" << std::endl;
	}
}

void createWindowSurface() {
	if (vkCreateRpiSurfaceEXT(instance, 0, 0, &windowSurface) != VK_SUCCESS) {
		std::cerr << "failed to create window surface!" << std::endl;
		assert(0);
	}

	std::cout << "created window surface" << std::endl;
}

void findPhysicalDevice() {
	// Try to find 1 Vulkan supported device
	// Note: perhaps refactor to loop through devices and find first one that supports all required features and extensions
	uint32_t deviceCount = 1;
	VkResult res = vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);
	if (res != VK_SUCCESS && res != VK_INCOMPLETE) {
		std::cerr << "enumerating physical devices failed!" << std::endl;
		assert(0);
	}

	if (deviceCount == 0) {
		std::cerr << "no physical devices that support vulkan!" << std::endl;
		assert(0);
	}

	std::cout << "physical device with vulkan support found" << std::endl;

	// Check device features
	// Note: will apiVersion >= appInfo.apiVersion? Probably yes, but spec is unclear.
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	uint32_t supportedVersion[] = {
		VK_VERSION_MAJOR(deviceProperties.apiVersion),
		VK_VERSION_MINOR(deviceProperties.apiVersion),
		VK_VERSION_PATCH(deviceProperties.apiVersion)
	};

	std::cout << "physical device supports version " << supportedVersion[0] << "." << supportedVersion[1] << "." << supportedVersion[2] << std::endl;
}

void checkSwapChainSupport() {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) {
		std::cerr << "physical device doesn't support any extensions" << std::endl;
		assert(0);
	}

	std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, deviceExtensions.data());

	for (const auto& extension : deviceExtensions) {
		if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
			std::cout << "physical device supports swap chains" << std::endl;
			return;
		}
	}

	std::cerr << "physical device doesn't support swap chains" << std::endl;
	assert(0);
}

void findQueueFamilies() {
	// Check queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	if (queueFamilyCount == 0) {
		std::cout << "physical device has no queue families!" << std::endl;
		assert(0);
	}

	// Find queue family with graphics support
	// Note: is a transfer queue necessary to copy vertices to the gpu or can a graphics queue handle that?
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::cout << "physical device has " << queueFamilyCount << " queue families" << std::endl;

	bool foundGraphicsQueueFamily = false;
	bool foundPresentQueueFamily = false;

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, windowSurface, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueFamily = i;
			foundGraphicsQueueFamily = true;

			if (presentSupport) {
				presentQueueFamily = i;
				foundPresentQueueFamily = true;
				break;
			}
		}

		if (!foundPresentQueueFamily && presentSupport) {
			presentQueueFamily = i;
			foundPresentQueueFamily = true;
		}
	}

	if (foundGraphicsQueueFamily) {
		std::cout << "queue family #" << graphicsQueueFamily << " supports graphics" << std::endl;

		if (foundPresentQueueFamily) {
			std::cout << "queue family #" << presentQueueFamily << " supports presentation" << std::endl;
		}
		else {
			std::cerr << "could not find a valid queue family with present support" << std::endl;
			assert(0);
		}
	}
	else {
		std::cerr << "could not find a valid queue family with graphics support" << std::endl;
		assert(0);
	}
}

void createLogicalDevice() {
	// Greate one graphics queue and optionally a separate presentation queue
	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo[2] = {};

	queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo[0].queueFamilyIndex = graphicsQueueFamily;
	queueCreateInfo[0].queueCount = 1;
	queueCreateInfo[0].pQueuePriorities = &queuePriority;

	queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo[0].queueFamilyIndex = presentQueueFamily;
	queueCreateInfo[0].queueCount = 1;
	queueCreateInfo[0].pQueuePriorities = &queuePriority;

	// Create logical device from physical device
	// Note: there are separate instance and device extensions!
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

	if (graphicsQueueFamily == presentQueueFamily) {
		deviceCreateInfo.queueCreateInfoCount = 1;
	}
	else {
		deviceCreateInfo.queueCreateInfoCount = 2;
	}

	const char* deviceExtensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions;

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
		std::cerr << "failed to create logical device" << std::endl;
		assert(0);
	}

	std::cout << "created logical device" << std::endl;

	// Get graphics and presentation queues (which may be the same)
	vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamily, 0, &presentQueue);

	std::cout << "acquired graphics and presentation queues" << std::endl;
}

void createSemaphores() {
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &createInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &createInfo, nullptr, &renderingFinishedSemaphore) != VK_SUCCESS) {
		std::cerr << "failed to create semaphores" << std::endl;
		assert(0);
	}
	else {
		std::cout << "created semaphores" << std::endl;
	}
}

void createSwapChain() {
	// Find surface capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, windowSurface, &surfaceCapabilities) != VK_SUCCESS) {
		std::cerr << "failed to acquire presentation surface capabilities" << std::endl;
		assert(0);
	}

	// Find supported surface formats
	uint32_t formatCount;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, windowSurface, &formatCount, nullptr) != VK_SUCCESS || formatCount == 0) {
		std::cerr << "failed to get number of supported surface formats" << std::endl;
		assert(0);
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, windowSurface, &formatCount, surfaceFormats.data()) != VK_SUCCESS) {
		std::cerr << "failed to get supported surface formats" << std::endl;
		assert(0);
	}

	// Find supported present modes
	uint32_t presentModeCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, windowSurface, &presentModeCount, nullptr) != VK_SUCCESS || presentModeCount == 0) {
		std::cerr << "failed to get number of supported presentation modes" << std::endl;
		assert(0);
	}

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, windowSurface, &presentModeCount, presentModes.data()) != VK_SUCCESS) {
		std::cerr << "failed to get supported presentation modes" << std::endl;
		assert(0);
	}

	// Determine number of images for swap chain
	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount != 0 && imageCount > surfaceCapabilities.maxImageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	std::cout << "using " << imageCount << " images for swap chain" << std::endl;

	// Select a surface format
	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(surfaceFormats);

	// Select swap chain size
	VkExtent2D swapChainExtent = chooseSwapExtent(surfaceCapabilities);

	// Check if swap chain supports being the destination of an image transfer
	// Note: AMD driver bug, though it would be nice to implement a workaround that doesn't use transfering
	//if (!(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
	//	std::cerr << "swap chain image does not support VK_IMAGE_TRANSFER_DST usage" << std::endl;
	//assert(0);
	//}

	// Determine transformation to use (preferring no transform)
	VkSurfaceTransformFlagBitsKHR surfaceTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		surfaceTransform = surfaceCapabilities.currentTransform;
	}

	// Choose presentation mode (preferring MAILBOX ~= triple buffering)
	VkPresentModeKHR presentMode = choosePresentMode(presentModes);

	// Finally, create the swap chain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = windowSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = surfaceTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		std::cerr << "failed to create swap chain" << std::endl;
		assert(0);
	}
	else {
		std::cout << "created swap chain" << std::endl;
	}

	// Store the images used by the swap chain
	// Note: these are the images that swap chain image indices refer to
	// Note: actual number of images may differ from requested number, since it's a lower bound
	uint32_t actualImageCount = 0;
	if (vkGetSwapchainImagesKHR(device, swapChain, &actualImageCount, nullptr) != VK_SUCCESS || actualImageCount == 0) {
		std::cerr << "failed to acquire number of swap chain images" << std::endl;
		assert(0);
	}

	swapChainImages.resize(actualImageCount);

	if (vkGetSwapchainImagesKHR(device, swapChain, &actualImageCount, swapChainImages.data()) != VK_SUCCESS) {
		std::cerr << "failed to acquire swap chain images" << std::endl;
		assert(0);
	}

	std::cout << "acquired swap chain images" << std::endl;
}

VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// We can either choose any format
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}

	// Or go with the standard format - if available
	for (const auto& availableSurfaceFormat : availableFormats) {
		if (availableSurfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
			return availableSurfaceFormat;
		}
	}

	// Or fall back to the first available one
	return availableFormats[0];
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
	if (surfaceCapabilities.currentExtent.width == -1) {
		VkExtent2D swapChainExtent = {};

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
		swapChainExtent.width = min(max(640, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width);
		swapChainExtent.height = min(max(480, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height);

		return swapChainExtent;
	}
	else {
		return surfaceCapabilities.currentExtent;
	}
}

VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> presentModes) {
	for (const auto& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	// If mailbox is unavailable, fall back to FIFO (guaranteed to be available)
	return VK_PRESENT_MODE_FIFO_KHR;
}

void createCommandQueues() {
	// Create presentation command pool
	// Note: only command buffers for a single queue family can be created from this pool
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = presentQueueFamily;

	if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		std::cerr << "failed to create command queue for presentation queue family" << std::endl;
		assert(0);
	}
	else {
		std::cout << "created command pool for presentation queue family" << std::endl;
	}

	// Get number of swap chain images and create vector to hold command queue for each one
	presentCommandBuffers.resize(swapChainImages.size());

	// Allocate presentation command buffers
	// Note: secondary command buffers are only for nesting in primary command buffers
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)swapChainImages.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, presentCommandBuffers.data()) != VK_SUCCESS) {
		std::cerr << "failed to allocate presentation command buffers" << std::endl;
		assert(0);
	}
	else {
		std::cout << "allocated presentation command buffers" << std::endl;
	}

	// Prepare data for recording command buffers
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	// Note: contains value for each subresource range
	VkClearColorValue clearColor = {
		{ 0.8f, 0.2f, 0.5f, 1.0f } // R, G, B, A
	};

	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.baseArrayLayer = 0;
	subResourceRange.layerCount = 1;

	// Record the command buffer for every swap chain image
	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		// Change layout of image to be optimal for clearing
		// Note: previous layout doesn't matter, which will likely cause contents to be discarded
		VkImageMemoryBarrier presentToClearBarrier = {};
		presentToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		presentToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		presentToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		presentToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		presentToClearBarrier.srcQueueFamilyIndex = presentQueueFamily;
		presentToClearBarrier.dstQueueFamilyIndex = presentQueueFamily;
		presentToClearBarrier.image = swapChainImages[i];
		presentToClearBarrier.subresourceRange = subResourceRange;

		// Change layout of image to be optimal for presenting
		VkImageMemoryBarrier clearToPresentBarrier = {};
		clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		clearToPresentBarrier.srcQueueFamilyIndex = presentQueueFamily;
		clearToPresentBarrier.dstQueueFamilyIndex = presentQueueFamily;
		clearToPresentBarrier.image = swapChainImages[i];
		clearToPresentBarrier.subresourceRange = subResourceRange;

		// Record command buffer
		vkBeginCommandBuffer(presentCommandBuffers[i], &beginInfo);

		vkCmdPipelineBarrier(presentCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);

		vkCmdClearColorImage(presentCommandBuffers[i], swapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subResourceRange);

		vkCmdPipelineBarrier(presentCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

		if (vkEndCommandBuffer(presentCommandBuffers[i]) != VK_SUCCESS) {
			std::cerr << "failed to record command buffer" << std::endl;
			assert(0);
		}
		else {
			std::cout << "recorded command buffer for image " << i << std::endl;
		}
	}
}

void draw() {
	// Acquire image
	uint32_t imageIndex;
	VkResult res = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		std::cerr << "failed to acquire image" << std::endl;
		assert(0);
	}

	std::cout << "acquired image" << std::endl;

	// Wait for image to be available and draw
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderingFinishedSemaphore;

	// This is the stage where the queue should wait on the semaphore (it doesn't have to wait with drawing, for example)
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &presentCommandBuffers[imageIndex];

	if (vkQueueSubmit(presentQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		std::cerr << "failed to submit draw command buffer" << std::endl;
		assert(0);
	}

	std::cout << "submitted draw command buffer" << std::endl;

	// Present drawn image
	// Note: semaphore here is not strictly necessary, because commands are processed in submission order within a single queue
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderingFinishedSemaphore;

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;

	res = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (res != VK_SUCCESS) {
		std::cerr << "failed to submit present command buffer" << std::endl;
		assert(0);
	}

	std::cout << "submitted presentation command buffer" << std::endl;
}

int main() {
	// Note: dynamically loading loader may be a better idea to fail gracefully when Vulkan is not supported

	// Create window for Vulkan
	//glfwInit();

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//window = glfwCreateWindow(640, 480, "The 630 line cornflower blue window", nullptr, nullptr);

	// Use Vulkan
	setupVulkan();

	mainLoop();

	cleanup();

	return 0;
}
