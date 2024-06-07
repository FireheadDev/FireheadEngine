#include "renderLoop.h"

#include <iostream>
#include <map>
#include <vector>

#include "QueueFamilyIndices.h"
#include "../logger/Logger.h"

const std::vector<const char*> RenderLoop::VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation",
};


RenderLoop::RenderLoop(const std::string& windowName, const std::string& appName, const int32_t& width, const int32_t& height)
{
	_windowName = windowName;
	_appName = appName;

	_windowWidth = width;
	_windowHeight = height;

	_window = nullptr;
	_instance = nullptr;

	_debugMessenger = nullptr;

	printf("Rendering Loop created\n");
}

void RenderLoop::Run()
{
	InitWindow();
	InitVulkan();

	MainLoop();

	Cleanup();
}

void RenderLoop::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(_windowWidth, _windowHeight, _windowName.data(), nullptr, nullptr);
}

void RenderLoop::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
	SelectPhysicalDevice();
}

void RenderLoop::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = Logger::VulkanDebugCallback;
	createInfo.pUserData = nullptr;
}

void RenderLoop::SetupDebugMessenger()
{
	if constexpr (!VALIDATION_LAYERS_ENABLED) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if(CreateDebugUtilsMessengerEXT(&createInfo, nullptr))
		throw std::runtime_error("Failed to set up debug messenger!");
}

void RenderLoop::GetExtensions(std::vector<const char*>& extensions)
{
	uint32_t extensionCount = 0;
	// Get required extensions for GLFW
	const char** glfwExtension = glfwGetRequiredInstanceExtensions(&extensionCount);

	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		extensions.push_back(glfwExtension[i]);
	}

	if (VALIDATION_LAYERS_ENABLED)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


	// Check all available extensions
	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	printf("Available Extensions:\n");
	for (const auto& extension : availableExtensions)
	{
		printf("\t%s\n", extension.extensionName);
	}
}

void RenderLoop::GetLayers(std::vector<VkLayerProperties>& layers)
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	layers.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
}

bool RenderLoop::ValidateLayerSupport(const std::vector<VkLayerProperties>& availableLayers)
{
	for (const char* layerName : VALIDATION_LAYERS)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

QueueFamilyIndices RenderLoop::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for(const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		if(indices.IsComplete())
			break;

		++i;
	}

	return indices;
}

int32_t RenderLoop::RateDeviceSuitability(const VkPhysicalDevice device)
{
	const QueueFamilyIndices indices = FindQueueFamilies(device);

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int32_t score = 0;

	// Ideal traits
	if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	score += static_cast<int32_t>(deviceProperties.limits.maxImageDimension2D);

	if(!deviceFeatures.geometryShader || !indices.IsComplete())
		return 0;

	return score;
}

void RenderLoop::SelectPhysicalDevice() const
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if(deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, availableDevices.data());

	std::multimap<int, VkPhysicalDevice> deviceCandidates;
	int32_t score;
	for(const auto& device : availableDevices)
	{
		score = RateDeviceSuitability(device);
		deviceCandidates.insert(std::make_pair(score, device));
	}

	if(deviceCandidates.rbegin()->first > 0)
		physicalDevice = deviceCandidates.rbegin()->second;
	else
		throw std::runtime_error("Failed to find a suitable GPU!");

}

void RenderLoop::CreateInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = _appName.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "No Engine... yet";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;


	// Extensions
	std::vector<const char*> extensions;
	GetExtensions(extensions);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Layers
	std::vector<VkLayerProperties> layers;
	GetLayers(layers);
	// ReSharper disable once CppRedundantBooleanExpressionArgument
	if (VALIDATION_LAYERS_ENABLED && !ValidateLayerSupport(layers))
	{
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	if (VALIDATION_LAYERS_ENABLED)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);
		createInfo.pNext = &debugMessengerCreateInfo;
	}
	else
	// ReSharper disable once CppUnreachableCode
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}


	if (const VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance); result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance! " + std::to_string(result));
	}
}

void RenderLoop::MainLoop() const
{
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
	}
}

void RenderLoop::Cleanup() const
{
	if(VALIDATION_LAYERS_ENABLED)
		(void)DestroyDebugUtilsMessengerEXT(nullptr);
	vkDestroyInstance(_instance, nullptr);

	glfwDestroyWindow(_window);
	glfwTerminate();
}


VkResult RenderLoop::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
{
	if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT")))  // NOLINT(clang-diagnostic-cast-function-type-strict)
		return func(_instance, pCreateInfo, pAllocator, &_debugMessenger);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VkResult RenderLoop::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) const
{
	if(const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT")))  // NOLINT(clang-diagnostic-cast-function-type-strict)
	{
		func(_instance, _debugMessenger, pAllocator);
		return VK_SUCCESS;
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}
