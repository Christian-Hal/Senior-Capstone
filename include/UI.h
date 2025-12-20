
#pragma once 
#include <glfw/glfw3.h>

class UI {

public: 
	void init(GLFWwindow* window);
	void draw();
	void shutdown();

private: 
	void drawPopup();
	void drawDrawEraseButton();
};
