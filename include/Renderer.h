
#pragma once 
//#include <glfw/glfw3.h>
#include "Globals.h"

class Renderer {

public:
	bool init(GLFWwindow* window, Globals g_inst);
	unsigned int beginFrame();
	void endFrame();
	void shutdown();
	void getFrameData();
	bool createFramebuffer(float fbWidth, float fbHeight);

private:
	int canvasWidth = 1920;
	int canvasHeight = 1080;

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int shaderProgram = 0; 

	unsigned int fbo = 0;
	unsigned int colorTexture;

	void newCanvas();
};