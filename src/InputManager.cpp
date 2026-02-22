#include "InputManager.h"

#include "Renderer.h"
#include "CanvasManager.h"
#include "UI.h"
#include "BrushManager.h"

#include "imgui.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CanvasManipulation.h"

Renderer* currRenderer = nullptr;
extern CanvasManager activeCanvasManager;

static UI ui;
extern Globals global;

// current, previous, and the delta of the x,y coordinates of the cursor
double currX, currY, lastX, lastY, deltaX, deltaY = 0;

//beginnings of the virtual key system
std::unordered_map<int, bool> CurrentKeys;
std::unordered_map<int, bool> PreviousKeys;

std::unordered_map<int, bool> CurrentMouse;
std::unordered_map<int, bool> PreviousMouse;


std::unordered_map<int, UI::CursorMode> KeyToCursorMode;
std::unordered_map<UI::CursorMode, int> CursorModeToKey;


CanvasManipulation canvasManipulation;

static bool WaitingForRebind = false;
static UI::CursorMode RebindTarget;
static bool RebindFailed = false;

// set the glfw callback funcitons up
void InputManager::init(GLFWwindow* window, Renderer* renderer)
{
	currRenderer = renderer;

	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetKeyCallback(window, keyboardCallBack);

	bindKey(UI::CursorMode::Rotate, GLFW_KEY_R);
	bindKey(UI::CursorMode::Pan, GLFW_KEY_H);
	bindKey(UI::CursorMode::Draw, GLFW_KEY_D);
	bindKey(UI::CursorMode::Erase, GLFW_KEY_E);
}

//constant update function
void InputManager::update()
{
	deltaX = currX - lastX;
	deltaY = currY - lastY;

	lastX = currX;
	lastY = currY;
}

// returns the true/false for the part of the mouse run through this function
// rn used for left click
bool InputManager::IsMousePressed(int button)
{
	return CurrentMouse[button];
}

// just return statements for potentially needed things for drawing
double InputManager::getMouseX() { return currX; }
double InputManager::getMouseY() { return currY; }

double InputManager::getMouseDeltaX() { return deltaX; }
double InputManager::getMouseDeltaY() { return deltaY; }

// callback function for the mouse buttons
void InputManager::mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (!currRenderer || ImGui::GetIO().WantCaptureMouse)
		return;

	// turning the respective mouse buttons to true on press and false on release
	//currently all functions used in the switch case work on every mouse click if in the right CursorMode,
	// changes needed in the future
	if (action == GLFW_PRESS) {
		CurrentMouse[button] = true;

		switch (ui.getCursorMode())
		{
		case UI::CursorMode::Rotate:
			canvasManipulation.startRotate(currX, currY);
			break;
		case UI::CursorMode::Pan:
			lastX = currX;
			lastY = currY;
			break;
		case UI::CursorMode::ZoomIn:
			canvasManipulation.zooming(1, 0.1, currX, currY, window);
			break;
		case UI::CursorMode::ZoomOut:
			canvasManipulation.zooming(-1, 0.1, currX, currY, window);
			break;
		}
	}

	else if (action == GLFW_RELEASE)
		CurrentMouse[button] = false;

}

// cursor x,y position callback, when left mouse button is pressed based on the mode, does the respective thing
// also consistenly updates the currX/currY with new x,y corrdinates when mouse moves
void InputManager::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	if (!currRenderer || ImGui::GetIO().WantCaptureMouse)
		return;

	if (IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		switch (ui.getCursorMode())
		{
		case UI::CursorMode::Pan:
			canvasManipulation.panning(deltaX, deltaY);
			break;
		case UI::CursorMode::Rotate:
			canvasManipulation.rotating(currX, currY);
			break;
		}
	}
	currX = xpos;
	currY = ypos;
}

// scoll wheel call back, used for mouse wheel scrolling and rotating when holding r key
void InputManager::scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	canvasManipulation.zooming(yoffset, 0.1, currX, currY, window);

}

// current hard code keyboardCallback, will be changing in the future when virtual keys are implemented
// changed the cursor mode in the ui when specific keys on the keyboard are pressed
// drag and zoom does not currently work
void InputManager::keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	if (action == GLFW_PRESS)
		CurrentKeys[key] = true;
	else if (action == GLFW_RELEASE)
		CurrentKeys[key] = false;

	// If waiting for rebind, capture this key
	if (WaitingForRebind && action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || bindKey(RebindTarget, key))
		{
			WaitingForRebind = false;
			RebindFailed = false;
		}
		else
		{
			RebindFailed = true;
		}

		return;
	}

	// Normal operation
	if (action == GLFW_PRESS)
	{
		auto temp = KeyToCursorMode.find(key);

		if (temp != KeyToCursorMode.end())
		{
			ui.setCursorMode(temp->second);
		}
	}

}


bool InputManager::bindKey(UI::CursorMode mode, int key)
{
	if (KeyToCursorMode.find(key) != KeyToCursorMode.end())
		return false;

	if (CursorModeToKey.find(mode) != CursorModeToKey.end())
	{
		int oldKey = CursorModeToKey[mode];
		KeyToCursorMode.erase(oldKey);
	}

	KeyToCursorMode[key] = mode;
	CursorModeToKey[mode] = key;
	return true;
}


void InputManager::StartRebind(UI::CursorMode mode)
{
	WaitingForRebind = true;
	RebindTarget = mode;
}


bool InputManager::IsWaitingForRebind()
{
	return WaitingForRebind;
}

bool InputManager::getRebindFail()
{
	return RebindFailed;
}