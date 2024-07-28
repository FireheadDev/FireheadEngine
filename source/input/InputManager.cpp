#include "InputManager.h"

#include <algorithm>

InputManager* InputManager::_instance = nullptr;

InputManager::InputManager(GLFWwindow* window)
{
	_window = window;
}

InputManager* InputManager::GetInstance(GLFWwindow* window)
{
	if (!_instance)
		_instance = new InputManager(window);
	return _instance;
}

/**
 *
 * @param window The GLFW window this input is coming from.
 * @param key The key-code of the key the event was triggered for
 * @param scancode The unique scancode for that physical key (warning, platform specific)
 * @param action The action the event represents (either GLFW_PRESS, GLFW_REPEAT, or GLFW_RELEASE)
 * @param mods Unused, but included to conform to the signature of GLFW's input event callback for simplicity.
 * @return A boolean representing if the input was processed by a callback that has been set (true), or if it fell through (false).
 */
bool InputManager::HandleInputEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN)
		return false;

	for (auto& listener : _listenerEvents)  // NOLINT(readability-use-anyofallof)
	{
		// Will need to change if input is switched to use scancodes instead of keycodes.
		if (listener.keyCode != key)
			continue;
		if (listener.trigger == FHE_TRIGGER_TYPE_PRESSED && action == GLFW_PRESS)
		{
			listener.callback(listener);
			return true;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_RELEASED && action == GLFW_RELEASE)
		{
			listener.callback(listener);
			return true;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_HELD)
		{
			if (action == GLFW_PRESS && !_heldListenerEvents.count(listener))
			{
				_heldListenerEvents.insert(listener);
				return true;
			}
			if (action == GLFW_RELEASE && _heldListenerEvents.count(listener))
			{
				_heldListenerEvents.erase(listener);
				return true;
			}
		}
	}

	return false;
}

void InputManager::HandleHeldEvents() const
{
	for(const auto& listenerEvents : _heldListenerEvents)
	{
		listenerEvents.callback(listenerEvents);
	}
}

void InputManager::AddListener(const InputListener& listener)
{
	_listenerEvents.push_back(listener);
}

bool InputManager::RemoveListener(const InputListener& listener)
{
	for (auto listenerEvent = _listenerEvents.begin(); listenerEvent != _listenerEvents.end(); ++listenerEvent)
	{
		if (listener == *listenerEvent)
		{
			_listenerEvents.erase(listenerEvent);
			return true;
		}
	}

	return false;
}
