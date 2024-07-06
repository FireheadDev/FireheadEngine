#include "logger.h"

#include <iostream>

Logger::Logger()
{
	printf("\n\n---Beginning Log---\n");
}

VkBool32 Logger::VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';


	return VK_FALSE;
}
