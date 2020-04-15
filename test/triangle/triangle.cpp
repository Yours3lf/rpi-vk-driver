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

//#define WINDOW_WIDTH 640
//#define WINDOW_HEIGHT 480

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
void CreateRenderPass();
void CreateFramebuffer();
void CreateShaders();
void CreatePipeline();
void CreateUniformBuffer();
void CreateDescriptorSet();
void CreateVertexBuffer();
void CreateTexture();
void recordCommandBuffers();
VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> presentModes);

VkInstance instance; //
VkSurfaceKHR windowSurface; //
VkPhysicalDevice physicalDevice;
VkDevice device; //
VkSemaphore imageAvailableSemaphore; //
VkSemaphore renderingFinishedSemaphore; //
VkSwapchainKHR swapChain; //
VkCommandPool commandPool; //
std::vector<VkCommandBuffer> presentCommandBuffers; //
std::vector<VkImage> swapChainImages; //
VkRenderPass renderPass; //
std::vector<VkFramebuffer> fbs; //
VkShaderModule shaderModule; //
VkPipeline pipeline; //
VkQueue graphicsQueue;
VkQueue presentQueue;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkPhysicalDeviceMemoryProperties pdmp;
std::vector<VkImageView> views; //?
VkSurfaceFormatKHR swapchainFormat;
VkExtent2D swapChainExtent;
VkPipelineLayout pipelineLayout;

uint32_t graphicsQueueFamily;
uint32_t presentQueueFamily;

void cleanup() {
	vkDeviceWaitIdle(device);

	// Note: this is done implicitly when the command pool is freed, but nice to know about
	vkFreeCommandBuffers(device, commandPool, presentCommandBuffers.size(), presentCommandBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderingFinishedSemaphore, nullptr);

	for(int c = 0; c < views.size(); ++c)
		vkDestroyImageView(device, views[c], 0);

	for (int c = 0; c < fbs.size(); ++c)
		vkDestroyFramebuffer(device, fbs[c], 0);

	vkDestroyRenderPass(device, renderPass, 0);

	vkDestroyShaderModule(device, shaderModule, 0);

	vkDestroyPipeline(device, pipeline, 0);

	// Note: implicitly destroys images (in fact, we're not allowed to do that explicitly)
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, windowSurface, nullptr);

	vkDestroyInstance(instance, nullptr);
}

void run() {
	// Note: dynamically loading loader may be a better idea to fail gracefully when Vulkan is not supported

	// Create window for Vulkan
	//glfwInit();

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "The 630 line cornflower blue window", nullptr, nullptr);

	// Use Vulkan
	setupVulkan();

	mainLoop();

	cleanup();
}

void setupVulkan() {
	createInstance();
	findPhysicalDevice();
	createWindowSurface();
	checkSwapChainSupport();
	findQueueFamilies();
	createLogicalDevice();
	createSemaphores();
	createSwapChain();
	createCommandQueues();
	CreateRenderPass();
	CreateFramebuffer();
	CreateVertexBuffer();
	//CreateUniformBuffer();
	CreateShaders();
	CreatePipeline();
	recordCommandBuffers();
}

void mainLoop() {
	//while (!glfwWindowShouldClose(window)) {
	for(int c = 0; c < 300; ++c){
		draw();

		//glfwPollEvents();
	}
}

void createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanTriangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "TriangleEngine";
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

	const char* enabledExtensions[] = {
		"VK_KHR_surface",
			"VK_KHR_display"
	};

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = sizeof(enabledExtensions) / sizeof(const char*);
	createInfo.ppEnabledExtensionNames = enabledExtensions;
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
	windowSurface = 0;

	/*PFN_vkCreateRpiSurfaceEXT vkCreateRpiSurfaceEXT = 0;
	vkCreateRpiSurfaceEXT = (PFN_vkCreateRpiSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateRpiSurfaceEXT");

	LoaderTrampoline* trampoline = (LoaderTrampoline*)physicalDevice;
	VkRpiPhysicalDevice* realPhysicalDevice = trampoline->loaderTerminator->physicalDevice;

	VkRpiSurfaceCreateInfoEXT ci = {};
	ci.pSurface = &windowSurface;

	realPhysicalDevice->customData = (uintptr_t)&ci;

	if (vkCreateRpiSurfaceEXT(physicalDevice) != VK_SUCCESS) {
		std::cerr << "failed to create window surface!" << std::endl;
		assert(0);
	}*/

	uint32_t displayCount;
	vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, 0);
	VkDisplayPropertiesKHR* displayProperties = (VkDisplayPropertiesKHR*)malloc(sizeof(VkDisplayPropertiesKHR)*displayCount);
	vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, displayProperties);

	uint32_t propertyCount;
	vkGetDisplayModePropertiesKHR(physicalDevice, displayProperties[0].display, &propertyCount, 0);
	VkDisplayModePropertiesKHR* displayModeProperties = (VkDisplayModePropertiesKHR*)malloc(sizeof(VkDisplayModePropertiesKHR)*propertyCount);
	vkGetDisplayModePropertiesKHR(physicalDevice, displayProperties[0].display, &propertyCount, displayModeProperties);

	VkDisplayModeCreateInfoKHR dmci = {};
	dmci.sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
	dmci.parameters = displayModeProperties[0].parameters;
	VkDisplayModeKHR displayMode;
	vkCreateDisplayModeKHR(physicalDevice, displayProperties[0].display, &dmci, 0, &displayMode);

	VkDisplaySurfaceCreateInfoKHR dsci = {};
	dsci.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	dsci.displayMode = displayMode;
	dsci.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	dsci.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
	dsci.imageExtent = displayModeProperties[0].parameters.visibleRegion;
	vkCreateDisplayPlaneSurfaceKHR(instance, &dsci, 0, &windowSurface);

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

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &pdmp);

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
	swapchainFormat = chooseSurfaceFormat(surfaceFormats);

	// Select swap chain size
	swapChainExtent = chooseSwapExtent(surfaceCapabilities);

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
	createInfo.imageFormat = swapchainFormat.format;
	createInfo.imageColorSpace = swapchainFormat.colorSpace;
	createInfo.imageExtent = swapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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
	views.resize(actualImageCount);

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
}

void recordCommandBuffers()
{
	// Prepare data for recording command buffers
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	// Note: contains value for each subresource range
	VkClearColorValue clearColor = {
		{ 0.4f, 0.6f, 0.9f, 1.0f } // R, G, B, A
	};
	VkClearValue clearValue = {};
	clearValue.color = clearColor;

	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.baseArrayLayer = 0;
	subResourceRange.layerCount = 1;

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent.width = swapChainExtent.width;
	renderPassInfo.renderArea.extent.height = swapChainExtent.height;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	VkViewport viewport = { 0 };
	viewport.height = (float)swapChainExtent.width;
	viewport.width = (float)swapChainExtent.height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;

	VkRect2D scissor = { 0 };
	scissor.extent.width = swapChainExtent.width;
	scissor.extent.height = swapChainExtent.height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	// Record the command buffer for every swap chain image
	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		// Record command buffer
		vkBeginCommandBuffer(presentCommandBuffers[i], &beginInfo);

		renderPassInfo.framebuffer = fbs[i];

		vkCmdBeginRenderPass(presentCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(presentCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		//vkCmdSetViewport(presentCommandBuffers[i], 0, 1, &viewport);

		//vkCmdSetScissor(presentCommandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(presentCommandBuffers[i], 0, 1, &vertexBuffer, &offsets );

		float Wcoeff = 1.0f; //1.0f / Wc = 2.0 - Wcoeff
		float viewportScaleX = (float)(swapChainExtent.width) * 0.5f * 16.0f;
		float viewportScaleY = -1.0f * (float)(swapChainExtent.height) * 0.5f * 16.0f;
		float Zs = 0.5f;

		uint32_t pushConstants[4];
		pushConstants[0] = *(uint32_t*)&Wcoeff;
		pushConstants[1] = *(uint32_t*)&viewportScaleX;
		pushConstants[2] = *(uint32_t*)&viewportScaleY;
		pushConstants[3] = *(uint32_t*)&Zs;

		vkCmdPushConstants(presentCommandBuffers[i], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), &pushConstants);

		vkCmdDraw(presentCommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(presentCommandBuffers[i]);

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

void CreateRenderPass()
{
	VkAttachmentReference attachRef = {};
	attachRef.attachment = 0;
	attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &attachRef;

	VkAttachmentDescription attachDesc = {};
	attachDesc.format = swapchainFormat.format; //Todo
	attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachDesc;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDesc;

	VkResult res = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);

	printf("Created a render pass\n");
}


void CreateFramebuffer()
{
	fbs.resize(swapChainImages.size());

	VkResult res;

	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo ViewCreateInfo = {};
		ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewCreateInfo.image = swapChainImages[i];
		ViewCreateInfo.format = swapchainFormat.format; //Todo
		ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ViewCreateInfo.subresourceRange.levelCount = 1;
		ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ViewCreateInfo.subresourceRange.layerCount = 1;

		res = vkCreateImageView(device, &ViewCreateInfo, NULL, &views[i]);

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = renderPass;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = &views[i];
		fbCreateInfo.width = swapChainExtent.width;
		fbCreateInfo.height = swapChainExtent.height;
		fbCreateInfo.layers = 1;

		res = vkCreateFramebuffer(device, &fbCreateInfo, NULL, &fbs[i]);
	}

	printf("Frame buffers created\n");
}

void CreateShaders()
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

	//clever: use small immedate -1 interpreted as 0xffffffff (white) to set color to white
	//"sig_small_imm ; tlb_color_all = or.always(b, b, nop, -1) ; nop = nop(r0, r0) ;"

	//8bit access
	//abcd
	//BGRA

	/**
	//rainbow colors
	char fs_asm_code[] =
			"sig_none ; r1 = itof.always(a, a, x_pix, uni) ; r3 = v8min.always(b, b) ;" //can't use mul pipeline for conversion :(
			"sig_load_imm ; r2 = load32.always(0x3a088888) ; nop = load32() ;" //1/1920
			"sig_none ; nop = nop(r0, r0) ; r2 = fmul.always(r2, r3);\n"
			"sig_none ; r1 = itof.pm.always(b, b, x_pix, y_pix) ; r0.8c = fmul.always(r1, r2) ;"
			"sig_load_imm ; r2 = load32.always(0x3a72b9d6) ; nop = load32() ;" //1/1080
			"sig_none ; nop = nop(r0, r0) ; r2 = fmul.always(r2, r3);\n"
			"sig_none ; nop = nop.pm(r0, r0) ; r0.8b = fmul.always(r1, r2) ;"
			"sig_small_imm ; nop = nop.pm(r0, r0, nop, 0) ; r0.8a = v8min.always(b, b) ;"
			"sig_small_imm ; nop = nop.pm(r0, r0, nop, 1) ; r0.8d = v8min.always(b, b) ;"
			"sig_none ; tlb_color_all = or.always(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_unlock_score ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
				"\0";
	/**/

	/**/
	//display a color
	char fs_asm_code[] =
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_load_imm ; r0 = load32.always(0xffa14ccc) ; nop = load32() ;"
			"sig_none ; tlb_color_all = or.always(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_end ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_none ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
			"sig_unlock_score ; nop = nop(r0, r0) ; nop = nop(r0, r0) ;"
				"\0";
	/**/

	char* asm_strings[] =
	{
		(char*)cs_asm_code, (char*)vs_asm_code, (char*)fs_asm_code, 0
	};

	VkRpiAssemblyMappingEXT mappings[] = {
		//vertex shader uniforms
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			0, //resource offset
			VK_SHADER_STAGE_VERTEX_BIT
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			4, //resource offset
			VK_SHADER_STAGE_VERTEX_BIT
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			8, //resource offset
			VK_SHADER_STAGE_VERTEX_BIT
		},
		{
			VK_RPI_ASSEMBLY_MAPPING_TYPE_PUSH_CONSTANT,
			VK_DESCRIPTOR_TYPE_MAX_ENUM, //descriptor type
			0, //descriptor set #
			0, //descriptor binding #
			0, //descriptor array element #
			12, //resource offset
			VK_SHADER_STAGE_VERTEX_BIT
		}
	};

	VkRpiShaderModuleAssemblyCreateInfoEXT shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.asmStrings = asm_strings;
	shaderModuleCreateInfo.mappings = mappings;
	shaderModuleCreateInfo.numMappings = sizeof(mappings) / sizeof(VkRpiAssemblyMappingEXT);
	shaderModuleCreateInfo.pShaderModule = &shaderModule;

	LoaderTrampoline* trampoline = (LoaderTrampoline*)physicalDevice;
	VkRpiPhysicalDevice* realPhysicalDevice = trampoline->loaderTerminator->physicalDevice;

	realPhysicalDevice->customData = (uintptr_t)&shaderModuleCreateInfo;

	PFN_vkCreateShaderModuleFromRpiAssemblyEXT vkCreateShaderModuleFromRpiAssemblyEXT = (PFN_vkCreateShaderModuleFromRpiAssemblyEXT)vkGetInstanceProcAddr(instance, "vkCreateShaderModuleFromRpiAssemblyEXT");

	VkResult res = vkCreateShaderModuleFromRpiAssemblyEXT(physicalDevice);
	assert(shaderModule);
}


#define VERTEX_BUFFER_BIND_ID 0

void CreatePipeline()
{
	VkPushConstantRange pushConstantRanges[1];
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = 4 * 4; //4 * 32bits
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 0;
	pipelineLayoutCI.pushConstantRangeCount = 1;
	pipelineLayoutCI.pPushConstantRanges = &pushConstantRanges[0];
	vkCreatePipelineLayout(device, &pipelineLayoutCI, 0, &pipelineLayout);


	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};

	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = shaderModule;
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = shaderModule;
	shaderStageCreateInfo[1].pName = "main";

	VkVertexInputBindingDescription vertexInputBindingDescription =
	{
		0,
		sizeof(float) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	VkVertexInputAttributeDescription vertexInputAttributeDescription =
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

	VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
	pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)swapChainExtent.width;
	vp.height = (float)swapChainExtent.height;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;

	VkPipelineViewportStateCreateInfo vpCreateInfo = {};
	vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCreateInfo.viewportCount = 1;
	vpCreateInfo.pViewports = &vp;

	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
	rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rastCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rastCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
	pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState blendAttachState = {};
	blendAttachState.colorWriteMask = 0xf;
	blendAttachState.blendEnable = false;

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = &blendAttachState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.depthTestEnable = false;
	depthStencilState.stencilTestEnable = false;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = &shaderStageCreateInfo[0];
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
	pipelineInfo.pViewportState = &vpCreateInfo;
	pipelineInfo.pRasterizationState = &rastCreateInfo;
	pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
	pipelineInfo.pColorBlendState = &blendCreateInfo;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.layout = pipelineLayout;

	VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline);

	printf("Graphics pipeline created\n");
}

uint32_t getMemoryTypeIndex(VkPhysicalDeviceMemoryProperties deviceMemoryProperties, uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	assert(0);
}

void CreateVertexBuffer()
{
	unsigned vboSize = sizeof(float) * 2 * 3; //3 x vec2

	VkMemoryRequirements mr;

	{ //create staging buffer
		VkBufferCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.size = vboSize;
		ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VkResult res = vkCreateBuffer(device, &ci, 0, &vertexBuffer);

		vkGetBufferMemoryRequirements(device, vertexBuffer, &mr);

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = mr.size;
		mai.memoryTypeIndex = getMemoryTypeIndex(pdmp, mr.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		res = vkAllocateMemory(device, &mai, 0, &vertexBufferMemory);

		float vertices[] =
		{
			-1, -1,
			1, -1,
			0, 1
		};

		void* data;
		res = vkMapMemory(device, vertexBufferMemory, 0, mr.size, 0, &data);
		memcpy(data, vertices, vboSize);
		vkUnmapMemory(device, vertexBufferMemory);

		res = vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
	}

	printf("Vertex buffer created\n");
}

int main() {
	// Note: dynamically loading loader may be a better idea to fail gracefully when Vulkan is not supported

	// Create window for Vulkan
	//glfwInit();

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "The 630 line cornflower blue window", nullptr, nullptr);

	// Use Vulkan
	setupVulkan();

	mainLoop();

	cleanup();


	return 0;
}
