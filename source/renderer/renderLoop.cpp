// ReSharper disable CppClangTidyBugproneUncheckedOptionalAccess
#include "renderLoop.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <memory>
#include <unordered_map>

#include "tiny_obj_loader.h"
#include "../input/InputManager.h"
#include "../logger/Logger.h"

const std::vector<const char*> RenderLoop::VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> RenderLoop::DEVICE_EXTENSIONS = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const std::string RenderLoop::SHADER_PATH = "shaders";
const std::string RenderLoop::MODEL_PATH = "models/trout_rainbow.obj";
const std::string RenderLoop::TEXTURE_PATH = "textures/trout_rainbow.png";

namespace
{
	std::vector<char> ReadFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open file!");

		const size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), static_cast<uint32_t>(fileSize));

		file.close();

		return buffer;
	}
}


RenderLoop::RenderLoop(const std::string& windowName, const std::string& appName, const int32_t& width, const int32_t& height)
{
	_windowName = windowName;
	_appName = appName;

	_windowWidth = width;
	_windowHeight = height;

	_window = nullptr;
	_cursor = nullptr;
	_instance = nullptr;
	_physicalDevice = nullptr;
	_device = nullptr;
	_surface = nullptr;
	_graphicsQueue = nullptr;
	_presentationQueue = nullptr;
	_transferQueue = nullptr;

	_swapChain = nullptr;
	_swapChainImageFormat = {};
	_swapChainExtent = {};

	_descriptorSetLayout = nullptr;
	_descriptorPool = nullptr;

	_renderPass = nullptr;
	_pipelineLayout = nullptr;
	_graphicsPipeline = nullptr;

	_commandPool = nullptr;
	_transferCommandPool = nullptr;

	_vertexBuffer = nullptr;
	_vertexBufferMemory = nullptr;
	_indexBuffer = nullptr;
	_indexBufferMemory = nullptr;

	_transformStagingData = nullptr;
	_transformBufferSize = 0;
	_transformStagingBuffer = nullptr;
	_transformStagingBufferMemory = nullptr;
	_transformBuffer = nullptr;
	_transformBufferMemory = nullptr;

	_depthImage = nullptr;
	_depthImageMemory = nullptr;
	_depthImageView = nullptr;

	_colorImage = nullptr;
	_colorImageMemory = nullptr;
	_colorImageView = nullptr;

	_debugMessenger = nullptr;

	_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(0);
	_currentFrame = 0;
	_frameBufferResized = false;

	_inputManager = nullptr;
	_camera = {};

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window = glfwCreateWindow(_windowWidth, _windowHeight, _windowName.data(), nullptr, nullptr);
	_cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	glfwSetWindowUserPointer(_window, this);
	glfwSetFramebufferSizeCallback(_window, FrameBufferResizeCallback);

	_inputManager = InputManager::GetInstance(_window, _cursor);
	glfwSetKeyCallback(_window, KeyCallback);
	glfwSetMouseButtonCallback(_window, MouseButtonCallback);
}

void RenderLoop::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	SelectPhysicalDevice();
	const QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_physicalDevice);
	CreateLogicalDevice(queueFamilyIndices);
	CreateSwapChain(queueFamilyIndices);
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateCommandPool(queueFamilyIndices);
	CreateDepthResources();
	CreateColorResources();
	CreateFrameBuffers();
	CreateTextures();
	LoadModels();
	SetupCamera();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateTransformBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void RenderLoop::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	const auto self = static_cast<RenderLoop*>(glfwGetWindowUserPointer(window));
	self->_frameBufferResized = true;
}

void RenderLoop::KeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
	const auto self = static_cast<RenderLoop*>(glfwGetWindowUserPointer(window));
	self->_inputManager->HandleKeyInputEvent(window, key, scancode, action, mods);
}

void RenderLoop::MouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
{
	const auto self = static_cast<RenderLoop*>(glfwGetWindowUserPointer(window));
	self->_inputManager->HandleMouseButtonInputEvent(window, button, action, mods);
}

void RenderLoop::CreateSurface()
{
	if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
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

void RenderLoop::CreateLogicalDevice(const QueueFamilyIndices& indices)
{
	if (!indices.IsComplete())
	{
		throw std::runtime_error("Failed to find the necessary queue families!");
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	const std::set uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

	float queuePriority = 1.f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
	deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	if (VALIDATION_LAYERS_ENABLED)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		deviceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else
		// ReSharper disable once CppUnreachableCode
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentationQueue);
	vkGetDeviceQueue(_device, indices.transferFamily.value(), 0, &_transferQueue);
}

void RenderLoop::CreateSwapChain(const QueueFamilyIndices& indices)
{
	const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
	// Only requesting the minimum image count can lead to waiting on the driver to complete operations, so an additional image is requested here
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queueFamilyIndices;
	GetUniqueQueueFamilyIndices(indices, queueFamilyIndices);
	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	createInfo.pQueueFamilyIndices = queueFamilyIndices.data();

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	// Allows for using alpha to blend with other windows in the window system??? May have to mess with this in a spike project some time.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	// Discards the pixels covered by another window
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}


	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
	_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

	_swapChainImageFormat = surfaceFormat.format;
	_swapChainExtent = extent;
}

void RenderLoop::CreateImageViews()
{
	_swapChainImageViews.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views!");
	}
}

void RenderLoop::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _swapChainImageFormat;
	colorAttachment.samples = _msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = _msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = _swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass!");
}

void RenderLoop::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding cameraBinding{};
	cameraBinding.binding = 0;
	cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraBinding.descriptorCount = 1;
	cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	cameraBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding transformBinding{};
	transformBinding.binding = 1;
	transformBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	transformBinding.descriptorCount = 1;
	transformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	transformBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 2;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.pImmutableSamplers = nullptr;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	const std::array<VkDescriptorSetLayoutBinding, 3> bindings = { cameraBinding, transformBinding, samplerBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");
}

void RenderLoop::CreateGraphicsPipeline()
{
	const auto vertShaderCode = ReadFile(RenderLoop::SHADER_PATH + "/shader_vert.spv");
	const auto fragShaderCode = ReadFile(RenderLoop::SHADER_PATH + "/shader_frag.spv");

	const VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	const VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	const std::array<VkDynamicState, 2> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateInfo.pDynamicStates = dynamicStates.data();

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescription();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(_swapChainExtent.width);
	viewport.height = static_cast<float>(_swapChainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	// TODO: Add support to dynamically enable wireframe rendering.
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.lineWidth = 1.f;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_TRUE;
	multisamplingInfo.rasterizationSamples = _msaaSamples;
	multisamplingInfo.minSampleShading = 0.2f;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.f;
	colorBlendInfo.blendConstants[1] = 0.f;
	colorBlendInfo.blendConstants[2] = 0.f;
	colorBlendInfo.blendConstants[3] = 0.f;

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.f;
	depthStencilInfo.maxDepthBounds = 1.f;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.front = {};
	depthStencilInfo.back = {};


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");


	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

void RenderLoop::CreateFrameBuffers()
{
	_swapChainFrameBuffers.resize(_swapChainImageViews.size());

	for (size_t i = 0; i < _swapChainImageViews.size(); ++i)
	{
		std::array<VkImageView, 3> attachments = {
			_colorImageView,
			_depthImageView,
			_swapChainImageViews[i],
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = _swapChainExtent.width;
		framebufferInfo.height = _swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFrameBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer!");
	}
}

void RenderLoop::CreateCommandPool(const QueueFamilyIndices& queueFamilyIndices)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");

	poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
	poolInfo.flags = poolInfo.flags | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_transferCommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create transfer command pool!");
}

void RenderLoop::CreateDepthResources()
{
	const VkFormat depthFormat = FindDepthFormat();

	CreateImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
	CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, _depthImageView);

	TransitionImageLayout(_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void RenderLoop::CreateColorResources()
{
	const VkFormat colorFormat = _swapChainImageFormat;

	CreateImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colorImage, _colorImageMemory);
	CreateImageView(_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, _colorImageView);
}


void RenderLoop::CreateTextures()
{
	_models.resize(1);
	_models[0].textures.resize(1);
	LoadTexture(TEXTURE_PATH, _models[0].textures[0].view, _models[0].textures[0].texture);
	CreateSampler(_models[0].textures[0]);
}

void RenderLoop::LoadModels()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
		throw std::runtime_error(warn + err);

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			vertex.color = { 1.f, 1.f, 1.f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(_models[0].vertices.size());
				_models[0].vertices.push_back(vertex);
			}
			_models[0].indices.push_back(uniqueVertices[vertex]);
		}
	}

	uint32_t transformCount = 0;
	for (auto& model : _models)
	{
		model.transformIndex = transformCount;
		_modelTransforms[&model] = std::make_shared<std::vector<glm::mat4>>();
		_modelTransforms[&model]->resize(FISH_WIDTH_COUNT * FISH_DEPTH_COUNT);

		for (size_t x = 0; x < FISH_WIDTH_COUNT; ++x)
		{
			for (size_t z = 0; z < FISH_DEPTH_COUNT; ++z)
			{
				glm::mat4 objectTransform
				{
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1,
				};
				objectTransform = glm::rotate(objectTransform, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
				objectTransform = glm::translate(objectTransform, glm::vec3(2 * x, 0, 2 * z));
				(*_modelTransforms[&model])[x * FISH_DEPTH_COUNT + z] = objectTransform;
				++transformCount;
			}
		}
	}
}

void RenderLoop::SetupCamera()
{
	_camera.view = lookAt(glm::vec3(0.f, 20.f, -15.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	_camera.projection = glm::perspective(glm::radians(45.f), static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height), 0.1f, 100.f);
	// Invert y coordinates for change from OpenGL to Vulkan
	_camera.projection[1][1] *= -1;
	_camera.speed = 15.f;

	InputListener listenerW{};
	listenerW.code = GLFW_KEY_W;
	listenerW.trigger = FHE_TRIGGER_TYPE_HELD;
	listenerW.callback = [this](const InputListener& listener)
		{
			_camera.view = glm::translate(_camera.view, glm::vec3(0.f, 0.f, _camera.speed) * _deltaTime.count());
		};
	InputListener listenerA{};
	listenerA.code = GLFW_KEY_A;
	listenerA.trigger = FHE_TRIGGER_TYPE_HELD;
	listenerA.callback = [this](const InputListener& listener)
		{
			_camera.view = glm::translate(_camera.view, glm::vec3(-_camera.speed, 0.f, 0.f) * _deltaTime.count());
		};
	InputListener listenerS{};
	listenerS.code = GLFW_KEY_S;
	listenerS.trigger = FHE_TRIGGER_TYPE_HELD;
	listenerS.callback = [this](const InputListener& listener)
		{
			_camera.view = glm::translate(_camera.view, glm::vec3(0.f, 0.f, -_camera.speed) * _deltaTime.count());
		};
	InputListener listenerD{};
	listenerD.code = GLFW_KEY_D;
	listenerD.trigger = FHE_TRIGGER_TYPE_HELD;
	listenerD.callback = [this](const InputListener& listener)
		{
			_camera.view = glm::translate(_camera.view, glm::vec3(_camera.speed, 0.f, 0.f) * _deltaTime.count());
		};

	_inputManager->AddKeyListener(listenerW);
	_inputManager->AddKeyListener(listenerA);
	_inputManager->AddKeyListener(listenerS);
	_inputManager->AddKeyListener(listenerD);
}

void RenderLoop::CreateVertexBuffer()
{
	const VkDeviceSize bufferSize = sizeof(_models[0].vertices[0]) * _models[0].vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, _models[0].vertices.data(), bufferSize);
	vkUnmapMemory(_device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);

	CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void RenderLoop::CreateIndexBuffer()
{
	const VkDeviceSize bufferSize = sizeof(_models[0].indices[0]) * _models[0].indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, _models[0].indices.data(), bufferSize);
	vkUnmapMemory(_device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);

	CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void RenderLoop::CreateTransformBuffer()
{
	for (auto model = _modelTransforms.begin(); model != _modelTransforms.end(); ++model)
	{
		_transformBufferSize += model->second->size();
	}
	_transformBufferSize *= sizeof(glm::mat4);

	CreateBuffer(_transformBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _transformStagingBuffer, _transformStagingBufferMemory);
	CreateBuffer(_transformBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _transformBuffer, _transformBufferMemory);

	vkMapMemory(_device, _transformStagingBufferMemory, 0, _transformBufferSize, 0, &_transformStagingData);
	CopyTransformsToDevice();
}

void RenderLoop::CreateUniformBuffers()
{
	// ReSharper disable once CppTooWideScope
	const VkDeviceSize bufferSize = sizeof(Camera);

	_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i], _uniformBuffersMemory[i]);

		vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
	}
}

void RenderLoop::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;


	if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");
}

void RenderLoop::CreateDescriptorSets()
{
	const std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
	allocInfo.pSetLayouts = layouts.data();

	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		std::array<VkDescriptorBufferInfo, 2> bufferInfo{};
		bufferInfo[0].buffer = _uniformBuffers[i];
		bufferInfo[0].offset = 0;
		bufferInfo[0].range = sizeof(Camera);
		bufferInfo[1].buffer = _transformBuffer;
		bufferInfo[1].offset = 0;
		bufferInfo[1].range = VK_WHOLE_SIZE;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// TODO: Temporarily hard-coded
		imageInfo.imageView = _models[0].textures[0].view;
		imageInfo.sampler = _models[0].textures[0].sampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo[0];  // NOLINT(readability-container-data-pointer)
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = _descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo[1];
		descriptorWrites[1].pImageInfo = nullptr;
		descriptorWrites[1].pTexelBufferView = nullptr;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = _descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = nullptr;
		descriptorWrites[2].pImageInfo = &imageInfo;
		descriptorWrites[2].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
}

void RenderLoop::CreateCommandBuffers()
{
	_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

	if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");
}

void RenderLoop::CreateSyncObjects()
{
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
	}
}

void RenderLoop::RecreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(_window, &width, &height);
	while (width == 0 || height == 0)
	{
		if (glfwWindowShouldClose(_window))
			return;

		glfwGetFramebufferSize(_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(_device);

	CleanupSwapChain();

	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_physicalDevice);
	CreateSwapChain(queueFamilyIndices);
	CreateImageViews();
	CreateDepthResources();
	CreateColorResources();
	CreateFrameBuffers();

	SetupCamera();
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

	if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr))
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



VkSurfaceFormatKHR RenderLoop::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// TODO: Add ranking in case of failure to find preferred format
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}

VkPresentModeKHR RenderLoop::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	// Guaranteed to be available 
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderLoop::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(_window, &width, &height);

	VkExtent2D actualExtents = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtents.width = std::clamp(actualExtents.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtents.height = std::clamp(actualExtents.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtents;
}

SwapChainSupportDetails RenderLoop::QuerySwapChainSupport(const VkPhysicalDevice device) const
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}



VkShaderModule RenderLoop::CreateShaderModule(const std::vector<char>& shaderCode) const
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");

	return shaderModule;
}

void RenderLoop::CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	// May need to change if not all buffers need to be concurrent
	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	std::vector<uint32_t> queueFamilyIndices{};
	GetUniqueQueueFamilyIndices(FindQueueFamilies(_physicalDevice), queueFamilyIndices);
	bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

	if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create buffer!");


	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate buffer memory!");

	vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

void RenderLoop::CopyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& size) const
{
	VkCommandBuffer commandBuffer;
	BeginSingleTimeCommand(commandBuffer);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void RenderLoop::CopyBufferToImage(const VkBuffer& buffer, const VkImage& image, const uint32_t& width, const uint32_t& height) const
{
	VkCommandBuffer commandBuffer;
	BeginSingleTimeCommand(commandBuffer);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(commandBuffer);
}

void RenderLoop::CopyTransformsToDevice()
{
	size_t offset = 0;
	for (auto model = _modelTransforms.begin(); model != _modelTransforms.end(); ++model)
	{
		const size_t innerSize = model->second->size() * sizeof(glm::mat4);
		memcpy(static_cast<char*>(_transformStagingData) + offset, model->second->data(), innerSize);
		offset += innerSize;
	}
	CopyBuffer(_transformStagingBuffer, _transformBuffer, _transformBufferSize);
}

void RenderLoop::CreateImage(const uint32_t& width, const uint32_t& height, const uint32_t& mipLevels, const VkSampleCountFlagBits& numSample, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkImage& image, VkDeviceMemory& imageMemory) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSample;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, image, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate image memory!");

	vkBindImageMemory(_device, image, imageMemory, 0);
}

void RenderLoop::CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectFlags, const uint32_t& mipLevels, VkImageView& imageView) const
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	// ReSharper disable once CppAssignedValueIsNeverUsed
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view!");
}

void RenderLoop::LoadTexture(std::string filePath, VkImageView& targetView, ktxVulkanTexture& targetTexture, const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkImageLayout& layout, const ktxTextureCreateFlagBits& createFlags) const
{
	ktxTexture* kTexture;
	ktxVulkanDeviceInfo vulkanDeviceInfo;
	if (ktxVulkanDeviceInfo_Construct(&vulkanDeviceInfo, _physicalDevice, _device, _graphicsQueue, _commandPool, nullptr) != KTX_error_code::KTX_SUCCESS)
		throw std::runtime_error("Could not construct vulkan device for the creation of KTX textures!");
	vulkanDeviceInfo.instance = _instance;

	if (filePath.find('.') == std::string::npos)
		throw std::runtime_error("Could not load a texture, because the filePath was invalid!");

	filePath = filePath.substr(0, filePath.find_last_of('.'));
	filePath.append(".ktx");
	if (ktxTexture_CreateFromNamedFile(filePath.c_str(), createFlags, &kTexture) != KTX_error_code::KTX_SUCCESS)
		throw std::runtime_error("Failed to create the texture from given file path!");

	kTexture->generateMipmaps = true;

	if (ktxTexture_VkUploadEx(kTexture, &vulkanDeviceInfo, &targetTexture, tiling, usage, layout))
		throw std::runtime_error("Failed to upload texture to the device! (consider checking the encoding format on the relevant .ktx file)");

	ktxTexture_Destroy(kTexture);
	ktxVulkanDeviceInfo_Destruct(&vulkanDeviceInfo);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
	};
	viewInfo.image = targetTexture.image;
	viewInfo.format = targetTexture.imageFormat;
	viewInfo.viewType = targetTexture.viewType;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = targetTexture.layerCount;
	viewInfo.subresourceRange.levelCount = targetTexture.levelCount;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;

	if (vkCreateImageView(_device, &viewInfo, nullptr, &targetView))
		throw std::runtime_error("Failed to create an image view for a texture!");
}

void RenderLoop::TransitionImageLayout(const VkImage& image, const VkFormat& format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const
	uint32_t& mipLevels) const
{
	VkCommandBuffer commandBuffer;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	BeginSingleTimeCommand(commandBuffer);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		throw std::invalid_argument("Unsupported layout transition!");


	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer);
}

void RenderLoop::CreateSampler(FHEImage& image) const
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	// TODO: Add parameters for the filters
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = static_cast<float>(image.texture.levelCount);

	if (vkCreateSampler(_device, &samplerInfo, nullptr, &image.sampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture sampler!");
}

void RenderLoop::BeginSingleTimeCommand(VkCommandBuffer& commandBuffer) const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void RenderLoop::EndSingleTimeCommands(const VkCommandBuffer& commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_graphicsQueue);

	vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}


QueueFamilyIndices RenderLoop::FindQueueFamilies(const VkPhysicalDevice& physicalDevice) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;
		else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			indices.transferFamily = i;

		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentationSupport);

		if (presentationSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
			break;

		++i;
	}

	return indices;
}

void RenderLoop::GetUniqueQueueFamilyIndices(const QueueFamilyIndices& indices, std::vector<uint32_t>& queueFamilyIndices)
{
	queueFamilyIndices = { indices.transferFamily.value(), indices.graphicsFamily.value() };
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{
		queueFamilyIndices.push_back(indices.presentFamily.value());
	}
}

int32_t RenderLoop::RateDeviceSuitability(const VkPhysicalDevice physicalDevice) const
{
	const QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	int32_t score = 0;

	// Ideal traits
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	score += static_cast<int32_t>(deviceProperties.limits.maxImageDimension2D);

	// Required traits
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
	const bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	if (!deviceFeatures.geometryShader || !indices.IsComplete() || !extensionsSupported || !swapChainAdequate || !supportedFeatures.samplerAnisotropy)
		return 0;

	return score;
}

bool RenderLoop::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void RenderLoop::SelectPhysicalDevice()
{

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, availableDevices.data());

	std::multimap<int, VkPhysicalDevice> deviceCandidates;
	int32_t score;
	for (const auto& device : availableDevices)
	{
		score = RateDeviceSuitability(device);
		deviceCandidates.insert(std::make_pair(score, device));
	}

	if (deviceCandidates.rbegin()->first > 0)
	{
		_physicalDevice = deviceCandidates.rbegin()->second;
		_msaaSamples = GetMaxUsableSampleCount();
		return;
	}

	throw std::runtime_error("Failed to find a suitable GPU!");
}

uint32_t RenderLoop::FindMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties) const
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

VkFormat RenderLoop::FindSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling& tiling, VkFormatFeatureFlags features) const
{
	for (const VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw std::runtime_error("Failed to find supported format!");
}

VkFormat RenderLoop::FindDepthFormat() const
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool RenderLoop::HasStencilComponent(const VkFormat& format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkSampleCountFlagBits RenderLoop::GetMaxUsableSampleCount() const
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

	const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}


void RenderLoop::RecordCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t& imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer!");


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = _swapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = _swapChainExtent;

	// Order must match the order of the attachments in CreateFrameBuffers
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.f, 0.f, 0.f, 1.f} };
	clearValues[1].depthStencil = { 1.f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(_swapChainExtent.width);
	viewport.height = static_cast<float>(_swapChainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	const VkBuffer vertexBuffers[] = { _vertexBuffer };
	const VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_currentFrame], 0, nullptr);

	for (auto& model : _models)
	{
		if (_modelTransforms[&model]->empty())
			continue;
		if (!RENDER_ONLY_FIRST_INSTANCE)
		{
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), static_cast<uint32_t>(_modelTransforms[&model]->size()), 0, 0, 0);
		}
		else
			// ReSharper disable once CppUnreachableCode
		{
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void RenderLoop::DrawFrame()
{
	vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swap chain image!");

	vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
	vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);
	RecordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);
	UpdateUniformBuffer();

	const VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
	const VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
	const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer!");


	const VkSwapchainKHR swapChains[] = { _swapChain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(_presentationQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _frameBufferResized)
	{
		_frameBufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderLoop::UpdateUniformBuffer()
{
	// Timing
	const auto currentTime = std::chrono::high_resolution_clock::now();
	_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastTime);
	_lastTime = currentTime;

	// Handle input (temporarily)
	_inputManager->HandleKeyHeldEvents();
	_inputManager->HandleMouseButtonHeldEvents();

	// Update transforms
	for (size_t i = 0; i < _modelTransforms[_models.data()]->size(); ++i)
	{
		(*_modelTransforms[_models.data()])[i] = rotate((*_modelTransforms[_models.data()])[i], _deltaTime.count() * glm::radians(-180.f), glm::vec3(0.f, 1.f, 0.f));
	}

	memcpy(_uniformBuffersMapped[_currentFrame], &_camera, sizeof(_camera));
	CopyTransformsToDevice();
}

void RenderLoop::MainLoop()
{
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(_device);
}

void RenderLoop::CleanupSwapChain() const
{
	vkDestroyImageView(_device, _depthImageView, nullptr);
	vkDestroyImage(_device, _depthImage, nullptr);
	vkFreeMemory(_device, _depthImageMemory, nullptr);

	vkDestroyImageView(_device, _colorImageView, nullptr);
	vkDestroyImage(_device, _colorImage, nullptr);
	vkFreeMemory(_device, _colorImageMemory, nullptr);

	for (const auto framebuffer : _swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(_device, framebuffer, nullptr);
	}
	for (const auto imageView : _swapChainImageViews)
	{
		vkDestroyImageView(_device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void RenderLoop::CleanupModels() const
{
	for (const auto& model : _models)
	{
		for (const auto& texture : model.textures)
		{
			vkDestroyImageView(_device, texture.view, nullptr);
			ktxVulkanTexture_Destruct(const_cast<ktxVulkanTexture*>(&texture.texture), _device, nullptr);
			vkDestroySampler(_device, texture.sampler, nullptr);
		}
	}
}

// TODO: Sort function into smaller functions like CleanupSwapChain to better label what is getting cleaned up and when.
void RenderLoop::Cleanup() const
{
	if (VALIDATION_LAYERS_ENABLED)
		(void)DestroyDebugUtilsMessengerEXT(nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
	}

	CleanupSwapChain();

	vkDestroyBuffer(_device, _vertexBuffer, nullptr);
	vkFreeMemory(_device, _vertexBufferMemory, nullptr);
	vkDestroyBuffer(_device, _indexBuffer, nullptr);
	vkFreeMemory(_device, _indexBufferMemory, nullptr);

	vkUnmapMemory(_device, _transformStagingBufferMemory);
	vkDestroyBuffer(_device, _transformStagingBuffer, nullptr);
	vkFreeMemory(_device, _transformStagingBufferMemory, nullptr);
	vkDestroyBuffer(_device, _transformBuffer, nullptr);
	vkFreeMemory(_device, _transformBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
		vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
	}

	// Textures
	CleanupModels();

	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
	vkDestroyRenderPass(_device, _renderPass, nullptr);

	vkDestroyCommandPool(_device, _commandPool, nullptr);
	vkDestroyCommandPool(_device, _transferCommandPool, nullptr);

	vkDestroyDevice(_device, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
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
	if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT")))  // NOLINT(clang-diagnostic-cast-function-type-strict)
	{
		func(_instance, _debugMessenger, pAllocator);
		return VK_SUCCESS;
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}
