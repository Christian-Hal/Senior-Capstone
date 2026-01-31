
#pragma once 
#include <glm/glm.hpp>


// Zooming and Panning around the canvas 

struct Camera2d
{
	glm::vec2 offset = { 0.0f, 0.0f };
	float zoom = 1.0f;
	float rotation = 0.0f;
};
