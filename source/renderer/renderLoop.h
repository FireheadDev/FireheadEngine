#ifndef RENDERER_RENDERLOOP_H_
#define RENDERER_RENDERLOOP_H_

#ifndef RENDERER_DLL
#define RENDERER_RENDERLOOP_API __declspec(dllexport)
#else
#define RENDERER_RENDERLOOP_API __declspec(dllimport)
#endif

extern "C"
{
	class RenderLoop
	{
	public:
		RENDERER_RENDERLOOP_API RenderLoop();
		RENDERER_RENDERLOOP_API void Run();
	private:
		RENDERER_RENDERLOOP_API void InitVulkan();
		RENDERER_RENDERLOOP_API void MainLoop();
		RENDERER_RENDERLOOP_API void Cleanup();
	};
}

#endif
