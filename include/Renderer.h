
#pragma once 
//#include <glfw/glfw3.h>

class Renderer {

public:
	bool init(GLFWwindow* window, int W, int H);
	unsigned int beginFrame();
	void endFrame();
	void shutdown();
	void getFrameData();

private:
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int shaderProgram = 0; 

	unsigned int fbo;
	unsigned int colorTexture;
};