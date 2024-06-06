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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../core/FHEMacros.h"

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

		VkDebugUtilsMessengerEXT _debugMessenger;

#pragma region Compile-Time Staic Members
		const static std::vector<const char*> VALIDATION_LAYERS;
		const static bool VALIDATION_LAYERS_ENABLED = IS_DEBUGGING_TERNARY(true, false);
#pragma endregion

#pragma region Initialization
		void InitWindow();
		void InitVulkan();

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetupDebugMessenger();
		static void GetExtensions(std::vector<const char*>& extensions);
		static void GetLayers(std::vector<VkLayerProperties>& layers);
		static bool ValidateLayerSupport(const std::vector<VkLayerProperties>& availableLayers);

		void SelectPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device) const;

		void CreateInstance();
#pragma endregion

		void MainLoop() const;
		void Cleanup() const;

#pragma region Extension Functions
		VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
		VkResult DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) const;
#pragma endregion
	};
}
#endif
