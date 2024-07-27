#ifndef INPUT_INPUTLISTENER_H_
#define INPUT_INPUTLISTENER_H_
#include "GLFW/glfw3.h"

#define INPUT_LISTENER_CALLBACK(Name) void (*(Name))(const InputListener&)

struct InputListener
{
	// GLFW key code to listen for
	int keyCode = GLFW_KEY_A;
	bool triggerOnPress = true;

	// Callback
	INPUT_LISTENER_CALLBACK(callback);

	bool operator==(const InputListener& other) const
	{
		return keyCode == other.keyCode && keyCode == triggerOnPress && callback == other.callback;
	}
};

#endif
