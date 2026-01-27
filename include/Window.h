
#pragma once 


#include "Globals.h"

#include <GLFW/glfw3.h>


class Window {

public:
	bool create(int width, int height, const char* title, Globals global);
	void pollEvents(); 
	void swapBuffers();
	bool shouldClose() const; 
	GLFWwindow* handle() const; 
	void destroy();  

private:
	GLFWwindow* m_window = nullptr;
};
