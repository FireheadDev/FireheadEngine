#ifndef INPUT_INPUTMANAGER_H_
#define INPUT_INPUTMANAGER_H_

#ifdef INPUT_DLL
#define INPUT_INPUTMANAGER_API __declspec(dllexport)
#else
#define INPUT_INPUTMANAGER_API __declspec(dllimport)
#endif

#include <set>

#include "InputListener.h"
#include "GLFW/glfw3.h"

extern "C"
{
	class InputManager
	{
	public:
		INPUT_INPUTMANAGER_API static InputManager* GetInstance(GLFWwindow* window, GLFWcursor* cursor);
#pragma region Key Events
		INPUT_INPUTMANAGER_API void HandleKeyInputEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
		INPUT_INPUTMANAGER_API void HandleKeyHeldEvents() const;
		INPUT_INPUTMANAGER_API void AddKeyListener(const InputListener& listener);
		INPUT_INPUTMANAGER_API bool RemoveKeyListener(const InputListener& listener);
#pragma endregion
#pragma region Mouse Events
		INPUT_INPUTMANAGER_API void HandleMouseButtonInputEvent(GLFWwindow* window, int button, int action, int mods);
		INPUT_INPUTMANAGER_API void HandleMouseButtonHeldEvents() const;
		INPUT_INPUTMANAGER_API void AddMouseButtonListener(const InputListener& listener);
		INPUT_INPUTMANAGER_API bool RemoveMouseButtonListener(const InputListener& listener);
#pragma endregion
	private:
		static InputManager* _instance;
		GLFWwindow* _window;
		GLFWcursor* _cursor;

		std::vector<InputListener> _keyListenerEvents;
		std::set<InputListener> _heldKeyListenerEvents;

		std::vector<InputListener> _mouseButtonListenerEvents;
		std::set<InputListener> _heldMouseButtonEvents;

		explicit InputManager(GLFWwindow* window, GLFWcursor* cursor);
	};
}

#endif
