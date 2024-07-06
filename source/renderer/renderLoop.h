#ifndef RENDERER_RENDERLOOP_H_
#define RENDERER_RENDERLOOP_H_

#ifndef RENDERER_DLL
#define RENDERER_RENDERLOOP_API __declspec(dllexport)
#else
#define RENDERER_RENDERLOOP_API __declspec(dllimport)
#endif

#include <cstdint>
#include <string>
#include <vector>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

#include "../core/FHEMacros.h"

struct SwapChainSupportDetails;
struct QueueFamilyIndices;

extern "C"
{
	class RenderLoop
	{
	public:
		RENDERER_RENDERLOOP_API explicit RenderLoop(const std::string& windowName, const std::string& appName, const int32_t& width = 800, const int32_t& height = 600);
		RENDERER_RENDERLOOP_API void Run();
	private:
		int32_t _windowWidth;
		int32_t _windowHeight;
		std::string _windowName;
		std::string _appName;

		GLFWwindow* _window;
		VkInstance _instance;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkQueue _graphicsQueue;
		VkQueue _presentationQueue;
		VkSwapchainKHR _swapChain;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkFramebuffer> _swapChainFramebuffers;

		VkRenderPass _renderPass;
		VkPipelineLayout _pipelineLayout;
		VkPipeline _graphicsPipeline;

		VkCommandPool _commandPool;
		std::vector<VkCommandBuffer> _commandBuffers;

		VkDebugUtilsMessengerEXT _debugMessenger;

		uint32_t _currentFrame;
		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;

#pragma region Compile-Time Staic Members
		const static std::vector<const char*> VALIDATION_LAYERS;
		const static bool VALIDATION_LAYERS_ENABLED = IS_DEBUGGING_TERNARY(true, false);
		const static std::vector<const char*> DEVICE_EXTENSIONS;
		const static uint32_t MAX_FRAMES_IN_FLIGHT = 2;
#pragma endregion

#pragma region Initialization
		void InitWindow();
		void InitVulkan();

		void CreateSurface();
		void CreateInstance();
		void CreateLogicalDevice(const VkPhysicalDevice& physicalDevice, const QueueFamilyIndices& indices);
		void CreateSwapChain(const VkPhysicalDevice& physicalDevice, const QueueFamilyIndices& indices);
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool(const QueueFamilyIndices& queueFamilyIndices);
		void CreateCommandBuffers();
		void CreateSyncObjects();

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetupDebugMessenger();
		static void GetExtensions(std::vector<const char*>& extensions);
		static void GetLayers(std::vector<VkLayerProperties>& layers);
		[[nodiscard]] static bool ValidateLayerSupport(const std::vector<VkLayerProperties>& availableLayers);
		[[nodiscard]] static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		[[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode) const;

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
		[[nodiscard]] int32_t RateDeviceSuitability(VkPhysicalDevice device) const;
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		[[nodiscard]] VkPhysicalDevice SelectPhysicalDevice() const;
#pragma endregion

		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t& imageIndex);
		void DrawFrame();
		void MainLoop();
		void Cleanup() const;

#pragma region Extension Functions
		VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
		VkResult DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) const;
#pragma endregion
	};
}
#endif
