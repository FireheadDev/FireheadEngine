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
#include <chrono>
#include <ktxvulkan.h>
#include <memory>

#include "Camera.h"
#include "FHEImage.h"
#include "Model.h"
#include "../core/FHEMacros.h"

class InputManager;
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
		GLFWcursor* _cursor;
		VkInstance _instance;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkQueue _graphicsQueue;
		VkQueue _presentationQueue;
		VkQueue _transferQueue;

		VkSwapchainKHR _swapChain;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkFramebuffer> _swapChainFrameBuffers;

		VkDescriptorSetLayout _descriptorSetLayout;
		VkDescriptorPool _descriptorPool;
		std::vector<VkDescriptorSet> _descriptorSets;

		VkRenderPass _renderPass;
		VkPipelineLayout _pipelineLayout;
		VkPipeline _graphicsPipeline;

		VkCommandPool _commandPool;
		VkCommandPool _transferCommandPool;
		std::vector<VkCommandBuffer> _commandBuffers;

		VkBuffer _vertexBuffer;
		VkDeviceMemory _vertexBufferMemory;
		VkBuffer _indexBuffer;
		VkDeviceMemory _indexBufferMemory;

		void* _transformStagingData;
		VkDeviceSize _transformBufferSize;
		VkBuffer _transformStagingBuffer;
		VkDeviceMemory _transformStagingBufferMemory;
		VkBuffer _transformBuffer;
		VkDeviceMemory _transformBufferMemory;

		std::vector<VkBuffer> _uniformBuffers;
		std::vector<VkDeviceMemory> _uniformBuffersMemory;
		std::vector<void*> _uniformBuffersMapped;

		// TODO: Create attachment image struct, potentially with some of the helper functions moved to that file instead of here.
		VkImage _depthImage;
		VkDeviceMemory _depthImageMemory;
		VkImageView _depthImageView;

		VkImage _colorImage;
		VkDeviceMemory _colorImageMemory;
		VkImageView _colorImageView;

		VkDebugUtilsMessengerEXT _debugMessenger;

		VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		// TODO: Move into separate timing class. Potentially move the semaphores and fences there as well?
		std::chrono::time_point<std::chrono::steady_clock> _lastTime;
		std::chrono::duration<float, std::chrono::seconds::period> _deltaTime;
		uint32_t _currentFrame;
		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;
		bool _frameBufferResized;

		// TODO: Move to a broader scope such as the app.
		InputManager* _inputManager;
		Camera _camera;

		std::vector<Model> _models;
		std::unordered_map<const Model*, std::shared_ptr<std::vector<glm::mat4>>> _modelTransforms;

#pragma region Compile-Time Static Members
		const static std::vector<const char*> VALIDATION_LAYERS;
		const static bool VALIDATION_LAYERS_ENABLED = IS_DEBUGGING_TERNARY(true, false);
		const static bool RENDER_ONLY_FIRST_INSTANCE = false;
		const static std::vector<const char*> DEVICE_EXTENSIONS;
		const static uint32_t MAX_FRAMES_IN_FLIGHT = 2;
		const static uint32_t FISH_WIDTH_COUNT = 11;
		const static uint32_t FISH_DEPTH_COUNT = 9;
		const static std::string SHADER_PATH;
		const static std::string MODEL_PATH;
		const static std::string TEXTURE_PATH;
#pragma endregion

#pragma region Initialization
		void InitWindow();
		void InitVulkan();

#pragma region Callbacks
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
#pragma endregion

		void CreateSurface();
		void CreateInstance();
		void CreateLogicalDevice(const QueueFamilyIndices& indices);
		void CreateSwapChain(const QueueFamilyIndices& indices);
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool(const QueueFamilyIndices& queueFamilyIndices);
		void CreateDepthResources();
		void CreateColorResources();
		void CreateTextures();
		void LoadModels();
		void SetupCamera();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateTransformBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void RecreateSwapChain();


		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetupDebugMessenger();
		static void GetExtensions(std::vector<const char*>& extensions);
		static void GetLayers(std::vector<VkLayerProperties>& layers);
		[[nodiscard]] static bool ValidateLayerSupport(const std::vector<VkLayerProperties>& availableLayers);

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

		[[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode) const;
		void CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		void CopyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& size) const;
		void CopyBufferToImage(const VkBuffer& buffer, const VkImage& image, const uint32_t& width, const uint32_t& height) const;
		void CopyTransformsToDevice();
		void CreateImage(const uint32_t& width, const uint32_t& height, const uint32_t& mipLevels, const VkSampleCountFlagBits& numSample, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkImage& image, VkDeviceMemory& imageMemory) const;
		void CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectFlags, const uint32_t& mipLevels, VkImageView& imageView) const;
		// TODO: Make parameters aside from the first 3 into a struct to simplify signature
		void LoadTexture(std::string filePath, VkImageView& targetView, ktxVulkanTexture& targetTexture, const VkImageTiling& tiling = VK_IMAGE_TILING_OPTIMAL, const VkImageUsageFlags& usage = VK_IMAGE_USAGE_SAMPLED_BIT, const VkImageLayout& layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, const ktxTextureCreateFlagBits& createFlags = KTX_TEXTURE_CREATE_NO_FLAGS) const;
		void TransitionImageLayout(const VkImage& image, const VkFormat& format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const uint32_t& mipLevels) const;
		void CreateSampler(FHEImage& image) const;

		void BeginSingleTimeCommand(VkCommandBuffer& commandBuffer) const;
		void EndSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice) const;
		static void GetUniqueQueueFamilyIndices(const QueueFamilyIndices& indices, std::vector<uint32_t>& queueFamilyIndices);
		[[nodiscard]] int32_t RateDeviceSuitability(VkPhysicalDevice physicalDevice) const;
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void SelectPhysicalDevice();
		[[nodiscard]] uint32_t FindMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties) const;
		[[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling& tiling, VkFormatFeatureFlags features) const;
		[[nodiscard]] VkFormat FindDepthFormat() const;
		[[nodiscard]] static bool HasStencilComponent(const VkFormat& format);
		[[nodiscard]] VkSampleCountFlagBits GetMaxUsableSampleCount() const;
#pragma endregion

#pragma region In Loop
		void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t& imageIndex);
		void DrawFrame();
		void UpdateUniformBuffer();
		void MainLoop();
#pragma endregion

#pragma region Cleanup
		void CleanupSwapChain() const;
		void CleanupModels() const;
		void Cleanup() const;
#pragma endregion

#pragma region Extension Functions
		VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
		VkResult DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) const;
#pragma endregion
	};
}
#endif
