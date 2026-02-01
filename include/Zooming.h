
#pragma once 
#include <glm/glm.hpp>


// Zooming and Panning around the canvas 

struct Camera2d
{
	glm::vec2 offset = { 0.0f, 0.0f };
	float zoom = 1.0f;
	float rotation = 0.0f;
};

static bool isPanning = false;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;

static float lastAngle = 0.0f;
static bool isZoomDragging = false;
static double lastZoomMouseY = 0.0;