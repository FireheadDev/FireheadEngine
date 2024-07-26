#ifndef RENDERER_CAMERA_H_
#define RENDERER_CAMERA_H_

#include <glm/glm.hpp>

struct Camera
{
	glm::mat4 view;
	glm::mat4 projection;
};

#endif