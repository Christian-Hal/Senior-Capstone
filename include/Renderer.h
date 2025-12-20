
#pragma once 
//#include <glfw/glfw3.h>

class Renderer {

public:
	bool init(GLFWwindow* window);
	void beginFrame();
	void endFrame();
	void shutdown();

private:
	unsigned int m_vao = 0;
	unsigned int m_vbo = 0; 
	unsigned int m_shaderProgram = 0; 
};