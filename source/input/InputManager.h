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
		INPUT_INPUTMANAGER_API static InputManager* GetInstance(GLFWwindow* window);
		INPUT_INPUTMANAGER_API [[nodiscard]] bool HandleInputEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
		INPUT_INPUTMANAGER_API void HandleHeldEvents() const;
		INPUT_INPUTMANAGER_API void AddListener(const InputListener& listener);
		INPUT_INPUTMANAGER_API bool RemoveListener(const InputListener& listener);
	private:
		static InputManager* _instance;
		GLFWwindow* _window;
		// TODO: Add support for mouse input
		std::vector<InputListener> _listenerEvents;
		std::set<InputListener> _heldListenerEvents;

		explicit InputManager(GLFWwindow* window);
	};
}

#endif
