
#pragma once 
#include <glfw/glfw3.h>
#include "Renderer.h"

class UI {

public: 
	void init(GLFWwindow* window);
	void draw();
	void shutdown();

private: 

	void drawPopup();

	void drawLeftPanel();
	void drawRightPanel();
	void drawTopPanel();
	void drawBottomPanel();
	void drawCenterCanvas();

	void drawDrawEraseButton();
	void drawColorWheel();
};
