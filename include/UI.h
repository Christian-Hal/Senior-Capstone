
#pragma once 

#include <functional>
#include <string>
#include <vector>

#include "Renderer.h"
#include "CanvasManager.h"
#include "Canvas.h"
#include "Globals.h"
#include "FrameRenderer.h"
#include "BrushTool.h"
#include "InputManager.h"

#include <GLFW/glfw3.h>

#include "CursorMode.h"

class UI {

public:
	using SetCursorModeCallback = std::function<void(CursorMode)>;
	using GetCursorModeCallback = std::function<CursorMode()>;
	using GetBrushListCallback = std::function<const std::vector<BrushTool>&()>;
	using SetActiveBrushCallback = std::function<void(int)>;
	using GetActiveBrushCallback = std::function<const BrushTool&()>;
	using LoadBrushCallback = std::function<void(const std::string&)>;
	using GetHotkeyLabelCallback = std::function<std::string(InputAction)>;
	using StartRebindCallback = std::function<void(InputAction)>;
	using BoolCallback = std::function<bool()>;
	using ResetCanvasPositionCallback = std::function<void()>;

	using GenerateBrushDabCallback = std::function<std::vector<float>(int)>; 



	// Lets the controller provide cursor state read/write hooks.
	// UI emits intent through these callbacks instead of owning app state.
    void bindCursorCallbacks(SetCursorModeCallback setCb, GetCursorModeCallback getCb);
	void bindBrushCallbacks(GetBrushListCallback getListCb, SetActiveBrushCallback setActiveCb, GetActiveBrushCallback getActiveCb, LoadBrushCallback loadBrushCb, 
		GenerateBrushDabCallback genDabCb);
	void bindHotkeyCallbacks(GetHotkeyLabelCallback getLabelCb, StartRebindCallback startCb, BoolCallback isWaitingCb, BoolCallback didFailCb);
	void bindCanvasCallbacks(ResetCanvasPositionCallback resetPositionCb);
	


	void init(GLFWwindow* window, Renderer& renderer, Globals& g_inst);
	void draw(CanvasManager& canvasManager, FrameRenderer frameRenderer);
	void shutdown();
	Color getColor();
	void setColor(Color pixelColor);
	void drawPopup(CanvasManager& canvasManager);
	int brushSize = 1; // default brush size

	CursorMode getCursorMode() const;
	void setCursorMode(CursorMode);

	static bool showPopup;

private:
	SetCursorModeCallback setCursorModeCb;
	GetCursorModeCallback getCursorModeCb;
	GetBrushListCallback getBrushListCb;
	SetActiveBrushCallback setActiveBrushCb;
	GetActiveBrushCallback getActiveBrushCb;
	LoadBrushCallback loadBrushFromFileCb;
	GetHotkeyLabelCallback getHotkeyLabelCb;
	StartRebindCallback startRebindCb;
	BoolCallback isWaitingForRebindCb;
	BoolCallback didRebindFailCb;
	ResetCanvasPositionCallback resetCanvasPositionCb;

	GenerateBrushDabCallback generateDabCb; 

	
	void drawCustomCursor(CanvasManager& canvasManager); 
	void drawLeftPanel(CanvasManager& canvasManager);
	void drawRightPanel(CanvasManager& canvasManager);
	void drawTopPanel(CanvasManager& canvasManager);
	void drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer);
	void drawCanvasTabs(CanvasManager& canvasManager);


};