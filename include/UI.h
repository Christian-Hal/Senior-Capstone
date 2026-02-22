
#pragma once 

#include <vector>

#include "Renderer.h"
#include "CanvasManager.h"
#include "Canvas.h"
#include "Globals.h"
#include "FrameRenderer.h"

#include <GLFW/glfw3.h>


class UI {

public:
	void init(GLFWwindow* window, Renderer& renderer, Globals& g_inst);
	void draw(CanvasManager& canvasManager, FrameRenderer frameRenderer);
	void shutdown();
	Color getColor();
	void drawPopup(CanvasManager& canvasManager);
	static int brushSize; 

	// enum 
	enum class CursorMode {
		Draw,
		Erase,
		ZoomIn,
		ZoomOut,
		Rotate,
		Pan,
		Rebind
	};

	CursorMode getCursorMode() const;
	//void UI::setCursorMode(UI::CursorMode);
	void setCursorMode(CursorMode);

private:
	void drawLeftPanel(CanvasManager& canvasManager);
	void drawRightPanel(CanvasManager& canvasManager);
	void drawTopPanel(CanvasManager& canvasManager);
	void drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer);

	void drawCanvasTabs(CanvasManager& canvasManager);
};