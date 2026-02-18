
#pragma once 

#include <vector>


#include "Globals.h"
#include "CanvasManager.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


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


class Renderer {

public:
	bool init(GLFWwindow* window, Globals& g_inst);
	void beginFrame(CanvasManager& canvasManager);
	void endFrame();
	void shutdown();

	// canvas rendering functions
	void createCanvasQuad(const Canvas& canvas);
	void uploadTexture(const Canvas& canvas);
	void renderCanvas(const Canvas& canvas);

	// color picking 
	void pickColor(double mouseX, double mouseY, Canvas& canvas); 


	//bool drawVertices = false;
	std::vector<float> drawVertices;
	bool isDrawing = false;

private:
	unsigned int canvasTexture = 0;
	int canvasWidth = 1920;
	int canvasHeight = 1080;

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int shaderProgram = 0; 

	unsigned int fbo = 0;
	unsigned int colorTexture;
};