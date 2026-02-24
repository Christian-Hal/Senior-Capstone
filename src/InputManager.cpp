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
CanvasManipulation canvasManipulation;

// current, previous, and the delta of the x,y coordinates of the cursor
double currX, currY, lastX, lastY, deltaX, deltaY = 0;

// map for the mouse buttons to see if pressed
std::unordered_map<int, bool> CurrentMouse;

// maps for key bindings look up and deletion
// the first map uses keycombo to use functions related to the InputAction
std::unordered_map<KeyCombo, std::function<void()>, KeyComboHash> KeyBindings;
std::unordered_map<InputAction, KeyCombo> ActionToKey;


// used for "turning on" rebind mode
static bool WaitingForRebind = false;

// used for ui left panel print if attempted keyCombo didn't work
static bool RebindFailed = false;

// InputAction attempting to rebind
static InputAction RebindTarget;

// set the glfw callback funcitons up
void InputManager::init(GLFWwindow* window, Renderer* renderer)
{
	currRenderer = renderer;

	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetKeyCallback(window, keyboardCallBack);


	bindAction(InputAction::setRotate, GLFW_KEY_R, 0);
	bindAction(InputAction::setPan, GLFW_KEY_H, 0);
	bindAction(InputAction::setDraw, GLFW_KEY_D, 0);
	bindAction(InputAction::setErase, GLFW_KEY_E, 0);
	bindAction(InputAction::undo, GLFW_KEY_Z, GLFW_MOD_CONTROL);
	bindAction(InputAction::redo, GLFW_KEY_X, GLFW_MOD_CONTROL);
	bindAction(InputAction::resetView, GLFW_KEY_R, GLFW_MOD_CONTROL);
	//bindAction(InputAction::setZoomDragging, GLFW_KEY_SPACE, GLFW_MOD_CONTROL);
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

	// turning the mouse buttons to true on press and false on release
	// depending on the cursormode currently selected, the respective operation will start
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

// keyboard call back to work with virtual key system
void InputManager::keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	// if statement that for rebind the hotkeys 
	if (WaitingForRebind && action == GLFW_PRESS)
	{
		// if ESC key is presed cancel rebind
		if (key == GLFW_KEY_ESCAPE)
		{
			WaitingForRebind = false;
			RebindFailed = false;
			return;
		}

		// this will not allow the modifyer keys to be selected alone to be hotkeys
		if (isModifierKey(key))
		{
			RebindFailed = true;
			return;
		}

		// calling the bind function to rebind the various actions
		if (bindAction(RebindTarget, key, mods))
		{
			WaitingForRebind = false;
			RebindFailed = false;
		}

		// if the atempted keybind is already set
		else
			RebindFailed = true;

		return;
	}

	// regular check statement to see if pressed keys have a key bind
	if (action == GLFW_PRESS)
	{
		KeyCombo combo{ key, mods };

		auto temp = KeyBindings.find(combo);

		if (temp != KeyBindings.end())
		{
			temp->second();
		}
	}

}

// binding/rebinding function for different Actions
bool InputManager::bindAction(InputAction action, int key, int mods)
{
	KeyCombo combo{ key, mods };

	// prevent duplicate binding, if combination of keys attempeted is already listed, that means 
	// there is already an that combination of keys connected to that action
	if (KeyBindings.find(combo) != KeyBindings.end())
		return false;

	// Removes the old key binding if this action already has one
	auto old = ActionToKey.find(action);
	if (old != ActionToKey.end())
		KeyBindings.erase(old->second);

	// each InputAction like setRotate, setPan, and etc. have a repective function to them
	// this set the new combination of keys (combo) to that function
	// undo and redo are just print statements to the terminal for now
	switch (action)
	{
	case InputAction::setRotate:
		KeyBindings[combo] = []() { ui.setCursorMode(UI::CursorMode::Rotate); };
		break;

	case InputAction::setPan:
		KeyBindings[combo] = []() { ui.setCursorMode(UI::CursorMode::Pan); };
		break;

	case InputAction::setDraw:
		KeyBindings[combo] = []() { ui.setCursorMode(UI::CursorMode::Draw); };
		break;

	case InputAction::setErase:
		KeyBindings[combo] = []() { ui.setCursorMode(UI::CursorMode::Erase); };
		break;

	case InputAction::undo:
		KeyBindings[combo] = []() { std::cout << "undo" << std::endl; };
		break;

	case InputAction::redo:
		KeyBindings[combo] = []() { std::cout << "redo" << std::endl; };
		break;

	case InputAction::resetView:
		KeyBindings[combo] = []() { canvasManipulation.centerCamera(); };
		break;

	//case InputAction::setZoomDragging:
	//	KeyBindings[combo] = []() { canvasManipulation.zoomDragging(deltaY, 0.005, currX, currY); };
	//	break;
	}

	ActionToKey[action] = combo;

	return true;
}

// takes in the action wanting rebind and turns the bool for rebind logic in keyboardCallBack true
void InputManager::StartRebind(InputAction action)
{
	WaitingForRebind = true;
	RebindTarget = action;
}

// returns the bool for seeing if rebinding is on for UI print
bool InputManager::IsWaitingForRebind()
{
	return WaitingForRebind;
}

// returns the bool for seeing if an invalid hotkey attemp was tried for UI print
bool InputManager::getRebindFail()
{
	return RebindFailed;
}

// keys that can be listed as modifyer keys that I didn't want to be able to make a hotkey
bool InputManager::isModifierKey(int key)
{
	return key == GLFW_KEY_LEFT_CONTROL ||
		key == GLFW_KEY_RIGHT_CONTROL ||
		key == GLFW_KEY_LEFT_SHIFT ||
		key == GLFW_KEY_RIGHT_SHIFT ||
		key == GLFW_KEY_LEFT_ALT ||
		key == GLFW_KEY_RIGHT_ALT ||
		key == GLFW_KEY_LEFT_SUPER ||
		key == GLFW_KEY_RIGHT_SUPER ||
		key == GLFW_KEY_ESCAPE ||
		key == GLFW_KEY_CAPS_LOCK ||
		key == GLFW_KEY_TAB ||
		key == GLFW_KEY_ENTER || 
		key == GLFW_KEY_INSERT ||
		key == GLFW_KEY_DELETE;
}


// takes in an input action, checks to see if there keycombo connected to it, 
// if so returns the string, if not returns Unbound
std::string InputManager::getHotkeyString(InputAction action)
{
	auto temp = ActionToKey.find(action);

	if (temp == ActionToKey.end())
		return "Unbound";

	return getKeybind(temp->second);
}

// converts the key combo given into string, available mod keys 
// are hard coded to be listed first with a "+" and following keys listed 
// after, if no mod keys are used, key string is returned alone
std::string InputManager::getKeybind(const KeyCombo& combo)
{
	std::string result;

	if (combo.mods & GLFW_MOD_CONTROL)
		result += "Ctrl+";

	if (combo.mods & GLFW_MOD_SHIFT)
		result += "Shift+";

	if (combo.mods & GLFW_MOD_ALT)
		result += "Alt+";

	if (combo.mods & GLFW_MOD_SUPER)
		result += "Super+";

	std::string keyName = glfwGetKeyName(combo.key, 0);
	
	keyName[0] = std::toupper(keyName[0]);
	result += keyName;

	return result;
}