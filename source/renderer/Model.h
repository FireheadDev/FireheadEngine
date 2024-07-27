#ifndef RENDERER_MODEL_H_
#define RENDERER_MODEL_H_
#include "FHEImage.h"
#include "Vertex.h"

struct Model
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<FHEImage> textures;

	uint32_t transformIndex;
};

#endif