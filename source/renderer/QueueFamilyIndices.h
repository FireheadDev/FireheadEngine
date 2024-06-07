#ifndef RENDERER_QUEUEFAMILYINDICES_H_
#define RENDERER_QUEUEFAMILYINDICES_H_

#include <cstdint>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value();
	}
};

#endif
