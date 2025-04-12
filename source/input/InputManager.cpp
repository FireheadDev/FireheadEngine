#include "InputManager.h"

#include <algorithm>

InputManager* InputManager::_instance = nullptr;

InputManager::InputManager(GLFWwindow* window, GLFWcursor* cursor)
{
	_window = window;
	_cursor = cursor;
}

// TODO: Add logic to support multiple instances with different windows (is multiple cursors even possible???)
InputManager* InputManager::GetInstance(GLFWwindow* window, GLFWcursor* cursor)
{
	if (!_instance)
		_instance = new InputManager(window, cursor);
	return _instance;
}

/**
 *
 * @param window The GLFW window this input is coming from.
 * @param key The key-code of the key the event was triggered for
 * @param scancode The unique scancode for that physical key (warning, platform specific)
 * @param action The action the event represents (either GLFW_PRESS, GLFW_REPEAT, or GLFW_RELEASE)
 * @param mods Unused, but included to conform to the signature of GLFW's input event callback for simplicity.
 */
void InputManager::HandleKeyInputEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN)
		return;

	for (auto& listener : _keyListenerEvents)  // NOLINT(readability-use-anyofallof)
	{
		// Will need to change if input is switched to use scancodes instead of keycodes.
		if (listener.code != key)
			continue;
		if (listener.trigger == FHE_TRIGGER_TYPE_PRESSED && action == GLFW_PRESS)
		{
			listener.callback(listener);
			return;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_RELEASED && action == GLFW_RELEASE)
		{
			listener.callback(listener);
			return;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_HELD)
		{
			if (action == GLFW_PRESS && !_heldKeyListenerEvents.count(listener))
			{
				_heldKeyListenerEvents.insert(listener);
				return;
			}
			if (action == GLFW_RELEASE && _heldKeyListenerEvents.count(listener))
			{
				_heldKeyListenerEvents.erase(listener);
				return;
			}
		}
	}
}

void InputManager::HandleKeyHeldEvents() const
{
	for(const auto& listenerEvents : _heldKeyListenerEvents)
	{
		listenerEvents.callback(listenerEvents);
	}
}

void InputManager::AddKeyListener(const InputListener& listener)
{
	_keyListenerEvents.push_back(listener);
}

bool InputManager::RemoveKeyListener(const InputListener& listener)
{
	for (auto listenerEvent = _keyListenerEvents.begin(); listenerEvent != _keyListenerEvents.end(); ++listenerEvent)
	{
		if (listener == *listenerEvent)
		{
			_keyListenerEvents.erase(listenerEvent);
			return true;
		}
	}

	return false;
}

void InputManager::HandleMouseButtonInputEvent(GLFWwindow* window, int button, int action, int mods)
{
	for (auto& listener : _mouseButtonListenerEvents)  // NOLINT(readability-use-anyofallof)
	{
		// Will need to change if input is switched to use scancodes instead of keycodes.
		if (listener.code != button)
			continue;
		if (listener.trigger == FHE_TRIGGER_TYPE_PRESSED && action == GLFW_PRESS)
		{
			listener.callback(listener);
			return;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_RELEASED && action == GLFW_RELEASE)
		{
			listener.callback(listener);
			return;
		}
		if (listener.trigger == FHE_TRIGGER_TYPE_HELD)
		{
			if (action == GLFW_PRESS && !_heldMouseButtonEvents.count(listener))
			{
				_heldMouseButtonEvents.insert(listener);
				return;
			}
			if (action == GLFW_RELEASE && _heldMouseButtonEvents.count(listener))
			{
				_heldMouseButtonEvents.erase(listener);
				return;
			}
		}
	}
}

void InputManager::HandleMouseButtonHeldEvents() const
{
	for(const auto& listenerEvents : _heldMouseButtonEvents)
	{
		listenerEvents.callback(listenerEvents);
	}
}

void InputManager::AddMouseButtonListener(const InputListener& listener)
{
	_mouseButtonListenerEvents.push_back(listener);
}

bool InputManager::RemoveMouseButtonListener(const InputListener& listener)
{
	for (auto listenerEvent = _mouseButtonListenerEvents.begin(); listenerEvent != _mouseButtonListenerEvents.end(); ++listenerEvent)
	{
		if (listener == *listenerEvent)
		{
			_mouseButtonListenerEvents.erase(listenerEvent);
			return true;
		}
	}

	return false;
}
