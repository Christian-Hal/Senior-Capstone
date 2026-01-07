
#pragma once 
#include <glfw/glfw3.h>
#include "Renderer.h"
#include "CanvasManager.h"
#include "Canvas.h"
#include "Globals.h"

class UI {

public: 
	void init(GLFWwindow* window, Renderer& renderer, Globals& g_inst);
	void draw(CanvasManager& canvasManager);
	void shutdown();

	void drawPopup(CanvasManager& canvasManager);

private: 
	void drawLeftPanel(CanvasManager& canvasManager);
	void drawRightPanel(CanvasManager& canvasManager);
	void drawTopPanel(CanvasManager& canvasManager);
	void drawBottomPanel(CanvasManager& canvasManager);


	void drawCenterCanvas(unsigned int colorTexture);
};
