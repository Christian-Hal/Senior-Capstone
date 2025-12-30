
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
	unsigned int m_vao = 0;
	unsigned int m_vbo = 0;
	unsigned int m_shaderProgram = 0; 

	unsigned int m_fbo;
	unsigned int m_colorTexture;
};