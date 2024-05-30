#include "renderLoop.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

RenderLoop::RenderLoop()
{
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

	_window = glfwCreateWindow(_windowWidth, _windowHeight, "Firehead Engine", nullptr, nullptr);
}

void RenderLoop::InitVulkan()
{
}

void RenderLoop::MainLoop()
{
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
	}
}

void RenderLoop::Cleanup()
{
	glfwDestroyWindow(_window);

	glfwTerminate();
}
