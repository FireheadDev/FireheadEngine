
#include <cstdlib>
#include <iostream>

#include "renderLoop.h"

int main()
{
	const std::string windowName{"Hello Triangle | FHE"};
	const std::string appName{"Hello Triangle"};

	try
	{
		RenderLoop renderingLoop = RenderLoop(windowName, appName);
		renderingLoop.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
