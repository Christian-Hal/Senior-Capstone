
#pragma once 
#include <glfw/glfw3.h>
#include "Renderer.h"

class UI {

public: 
	void init(GLFWwindow* window, Renderer renderer);
	void draw(unsigned int colorTexture);
	void shutdown();

private: 

	void drawPopup();

	void drawLeftPanel();
	void drawRightPanel();
	void drawTopPanel();
	void drawBottomPanel();
	void drawCenterCanvas(unsigned int colorTexture);

	void drawDrawEraseButton();
	void drawColorWheel();
};
