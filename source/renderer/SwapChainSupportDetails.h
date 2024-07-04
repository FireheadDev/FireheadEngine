#ifndef RENDERER_SWAPCHAINSUPPORTDETAILS_H_
#define RENDERER_SWAPCHAINSUPPORTDETAILS_H_

#include <vector>

#include "vulkan/vulkan.h"

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

#endif