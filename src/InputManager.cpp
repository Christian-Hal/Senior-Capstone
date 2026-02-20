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
std::unordered_map<int, bool> s_CurrentKeys;
std::unordered_map<int, bool> s_PreviousKeys;

std::unordered_map<int, bool> s_CurrentMouse;
std::unordered_map<int, bool> s_PreviousMouse;


CanvasManipulation canvasManipulation;


// set the glfw callback funcitons up
void InputManager::init(GLFWwindow* window, Renderer* renderer)
{
	currRenderer = renderer;

    glfwSetMouseButtonCallback(window, mouseButtonCallBack);
    glfwSetCursorPosCallback(window, cursorPosCallBack);
    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetKeyCallback(window, keyboardCallBack);
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
	return s_CurrentMouse[button];
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
		s_CurrentMouse[button] = true;

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
		s_CurrentMouse[button] = false;
	
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

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Rotate);
		return;
	}

	else if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Pan);
		return;
	}

	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Draw);
		return;
	}

	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Erase);
		return;
	}

	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS &&
			(mods & GLFW_MOD_CONTROL))
		{
			//isZoomDragging = true;
			//glfwGetCursorPos(window, nullptr, &lastZoomMouseY);
		}

		if (action == GLFW_RELEASE)
		{
			//isZoomDragging = false;
		}
	}

	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
	{
		//if (action == GLFW_RELEASE)
			//isZoomDragging = false;
	
	}
}



