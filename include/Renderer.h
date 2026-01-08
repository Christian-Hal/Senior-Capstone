
#pragma once 
//#include <glfw/glfw3.h>
#include "Globals.h"
#include "CanvasManager.h"
#include <vector>

class Renderer {

public:
	bool init(GLFWwindow* window, Globals& g_inst);
	void beginFrame(CanvasManager& canvasManager);
	void endFrame();
	void shutdown();
	void getFrameData(CanvasManager& canvasManagerA);
	bool createFramebuffer(float fbWidth, float fbHeight);

	// canvas rendering functions
	void createCanvasQuad(const Canvas& canvas);
	void uploadTexture(const Canvas& canvas);
	void renderCanvas(const Canvas& canvas);

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