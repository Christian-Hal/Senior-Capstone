
#pragma once 
#include <glfw/glfw3.h>

class Window {

public:
	bool create(int width, int height, const char* title);
	void pollEvents(); 
	void swapBuffers();
	bool shouldClose() const; 
	GLFWwindow* handle() const; 
	void destroy(); 

private:
	GLFWwindow* m_window = nullptr;
};
