#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#ifndef LOGGER_DLL
#define LOGGER_LOGGER_API __declspec(dllexport)
#else
#define LOGGER_LOGGER_API __declspec(dllimport)
#endif

#include "vulkan/vulkan.h"

extern "C"
{
	class Logger
	{
	private:
	public:
		LOGGER_LOGGER_API Logger();
		LOGGER_LOGGER_API static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};
}

#endif
