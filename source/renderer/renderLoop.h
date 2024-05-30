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
	private:
	public:
		RENDERER_RENDERLOOP_API RenderLoop();
	};
}

#endif
