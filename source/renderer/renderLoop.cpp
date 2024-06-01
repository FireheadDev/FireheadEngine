#include "renderLoop.h"

#include <iostream>
#include <vector>

RenderLoop::RenderLoop(const std::string& windowName, const std::string& appName, const int32_t& width, const int32_t& height)
{
	_windowName = windowName;
	_appName = appName;

	_windowWidth = width;
	_windowHeight = height;

	_window = nullptr;
	_instance = nullptr;
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
}

void RenderLoop::GetExtensions(uint32_t& extensionCount, std::vector<const char*>& extensions)
{
	// Get required extensions for GLFW
	const char** glfwExtension = glfwGetRequiredInstanceExtensions(&extensionCount);

	for(uint32_t i = 0; i < extensionCount; ++i)
	{
		extensions.push_back(glfwExtension[i]);
	}


	// Check all available extensions
	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	printf("Available Extensions:\n");
	for(const auto& extension : availableExtensions)
	{
		printf("\t%s\n", extension.extensionName);
	}
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

	uint32_t extensionCount = 0;
	std::vector<const char*> extensions;
	GetExtensions(extensionCount, extensions);

	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;


	if(const VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance); result != VK_SUCCESS)
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
	vkDestroyInstance(_instance, nullptr);

	glfwDestroyWindow(_window);
	glfwTerminate();
}
