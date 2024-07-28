#include "InputListener.h"

bool InputListener::operator==(const InputListener& other) const noexcept
{
	// TODO: Add comparison of the functions
	return keyCode == other.keyCode && trigger == other.trigger && callback.target<FHE_INPUT_CALLBACK_TYPE>() == other.callback.target<FHE_INPUT_CALLBACK_TYPE>();
}

bool InputListener::operator<(const InputListener& other) const noexcept
{
	return std::hash<InputListener>{}(*this) < std::hash<InputListener>{}(other);
}
