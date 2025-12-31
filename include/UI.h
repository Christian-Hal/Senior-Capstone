
#pragma once 
#include <glfw/glfw3.h>
#include "Renderer.h"
#include "Globals.h"

class UI {

public: 
	void init(GLFWwindow* window, Renderer renderer, Globals g_inst);
	void draw();
	void shutdown();

	void drawPopup();

private: 
	void drawLeftPanel();
	void drawRightPanel();
	void drawTopPanel();
	void drawBottomPanel();
	void drawCenterCanvas(unsigned int colorTexture);
};
