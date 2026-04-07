
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
	using GenerateBrushDabCallback = std::function<std::vector<float>(int)>; 
	using GetHotkeyLabelCallback = std::function<std::string(InputAction)>;
	using StartRebindCallback = std::function<void(InputAction)>;
	using BoolCallback = std::function<bool()>;
	using ResetCanvasPositionCallback = std::function<void()>;
	using saveToRecentActivityCallback = std::function<void(const std::string&)>;
	using getRecentActivityCallback = std::function<const std::vector<std::string>&()>;
	using getDefaultFolderPathCallback = std::function<std::string()>;


	// Lets the controller provide cursor state read/write hooks.
	// UI emits intent through these callbacks instead of owning app state.
    void bindCursorCallbacks(SetCursorModeCallback setCb, GetCursorModeCallback getCb);
	void bindBrushCallbacks(GetBrushListCallback getListCb, SetActiveBrushCallback setActiveCb, GetActiveBrushCallback getActiveCb, LoadBrushCallback loadBrushCb, 
		GenerateBrushDabCallback genDabCb);
	void bindHotkeyCallbacks(GetHotkeyLabelCallback getLabelCb, StartRebindCallback startCb, BoolCallback isWaitingCb, BoolCallback didFailCb);
	void bindCanvasCallbacks(ResetCanvasPositionCallback resetPositionCb);
	void bindRecentActivityCallbacks(saveToRecentActivityCallback saveCb, getRecentActivityCallback getCb);
	void bindDefaultFolderPathCallback(getDefaultFolderPathCallback getDefaultFolderPathCb);

	// ui drawing functions
	void drawUI(CanvasManager& canvasManager, FrameRenderer frameRenderer);

	// other functions
	void init(GLFWwindow* window, Renderer& renderer, Globals& g_inst);
	void shutdown();
	Color getColor();
	void setColor(Color pixelColor);
	int brushSize = 1; // default brush size

	CursorMode getCursorMode() const;
	void setCursorMode(CursorMode);

	// ui state stuff
	enum class UIState {
		start_menu,
		main_screen
	};

private:
	SetCursorModeCallback setCursorModeCb;
	GetCursorModeCallback getCursorModeCb;

	GetBrushListCallback getBrushListCb;
	SetActiveBrushCallback setActiveBrushCb;
	GetActiveBrushCallback getActiveBrushCb;
	LoadBrushCallback loadBrushFromFileCb;
	GenerateBrushDabCallback generateDabCb; 

	GetHotkeyLabelCallback getHotkeyLabelCb;
	StartRebindCallback startRebindCb;
	BoolCallback isWaitingForRebindCb;
	BoolCallback didRebindFailCb;

	ResetCanvasPositionCallback resetCanvasPositionCb;
	saveToRecentActivityCallback saveToRecentActivityCb;
	getRecentActivityCallback getRecentActivityCb;
	getDefaultFolderPathCallback getDefaultFolderPathCb;

	// main UI drawing functions for each state
	void drawStartScreen(CanvasManager& canvasManager);
	void drawMainScreen(CanvasManager& canvasManager, FrameRenderer frameRenderer);

	// helper functions for drawing different panels / popups
	void drawCustomCursor(CanvasManager& canvasManager); 
	void drawLeftPanel(CanvasManager& canvasManager);
	void drawRightPanel(CanvasManager& canvasManager);
	void drawTopPanel(CanvasManager& canvasManager);
	void drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer);
	void drawCanvasTabs(CanvasManager& canvasManager);

	void drawNewCanvasPopup(CanvasManager& canvasManager);

	// keeps track of the UI's current state so we know what stuff to draw
	UIState curState = UIState::start_menu;

	// tracks the window display size for the ui panel sizing calculations
	float displayWidth;
	float displayHeight;
};