#ifndef RENDERER_QUEUEFAMILYINDICES_H_
#define RENDERER_QUEUEFAMILYINDICES_H_

#include <cstdint>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	[[nodiscard]] bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

#endif
