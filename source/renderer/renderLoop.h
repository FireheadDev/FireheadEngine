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

		RENDERER_RENDERLOOP_API void InitWindow();
		RENDERER_RENDERLOOP_API void InitVulkan();
		static void GetExtensions(uint32_t& extensionCount, std::vector<const char*>& extensions);
		RENDERER_RENDERLOOP_API void CreateInstance();
		RENDERER_RENDERLOOP_API void MainLoop() const;
		RENDERER_RENDERLOOP_API void Cleanup() const;
	};
}
#endif
