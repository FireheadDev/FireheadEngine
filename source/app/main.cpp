
#include <cstdlib>
#include <iostream>

#include "renderLoop.h"

void HandleEnd();

int main()
{
	const std::string windowName{ "Weird Fishes | FHE" };
	const std::string appName{ "Weird Fishes" };

	try
	{
		RenderLoop renderingLoop = RenderLoop(windowName, appName);
		renderingLoop.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		HandleEnd();
		return EXIT_FAILURE;
	}

	HandleEnd();
	return EXIT_SUCCESS;
}

void HandleEnd()
{
#ifndef NDEBUG
	printf("Press any key to continue...");
	(void)std::getchar();
#endif
}

