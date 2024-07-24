#ifndef RENDERER_FHEIMAGE_H_
#define RENDERER_FHEIMAGE_H_

#include <vulkan/vulkan.h>

#include "ktxvulkan.h"

struct FHEImage
{
	uint32_t mipLevels;
	VkImageView view;
	ktxVulkanTexture texture;
	VkSampler sampler;
};

#endif