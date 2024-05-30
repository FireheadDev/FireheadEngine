
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "renderLoop.h"

int main()
{
	try
	{
		RenderLoop renderingLoop;
		renderingLoop.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
