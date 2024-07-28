#ifndef INPUT_INPUTLISTENER_H_
#define INPUT_INPUTLISTENER_H_

#include <functional>

#include "GLFW/glfw3.h"
#include "FHETriggerType.h"

#define FHE_INPUT_CALLBACK_TYPE void(const InputListener&)

struct InputListener
{
	// GLFW key code to listen for
	int keyCode = GLFW_KEY_A;
	FHETriggerType trigger;

	// Callback
	std::function<FHE_INPUT_CALLBACK_TYPE> callback;

	bool operator==(const InputListener& other) const noexcept;

	bool operator<(const InputListener& other) const noexcept;
};

template<> struct std::hash<InputListener>
{
	size_t operator()(InputListener const& inputListener) const noexcept
	{
		return (std::hash<int>{}(inputListener.keyCode) ^
			(std::hash<FHETriggerType>{}(inputListener.trigger) << 1) >> 1) ^
			std::hash<size_t>{}(reinterpret_cast<size_t>(inputListener.callback.target<FHE_INPUT_CALLBACK_TYPE>())) << 1;
	}
};

#endif
