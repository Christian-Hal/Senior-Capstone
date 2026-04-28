#include "InputManager.h"

#include "imgui.h"

#include <json.hpp>
#include <fstream>
using json = nlohmann::json;

#include <cctype>
#include <string>
#include <unordered_map>
#include <utility>

// Mouse event callback functions
static InputManager::MouseMoveCallback mouseMoveCallback;
static InputManager::MouseButtonCallback mouseButtonCallback;
static InputManager::MouseScrollCallback mouseScrollCallback;
static InputManager::InputActionCallback inputActionCallback;

// current, previous, and the delta of the x,y coordinates of the cursor
double currX = 0; 
double currY = 0;
double lastX = 0; 
double lastY = 0; 
double deltaX = 0; 
double deltaY = 0;

// map for the mouse buttons to see if pressed
std::unordered_map<int, bool> CurrentMouse;

// uses the current mouse information to build a MouseState struct to give to AppController
static MouseState buildMouseState()
{
	return MouseState{
		currX,
		currY,
		deltaX,
		deltaY,
		CurrentMouse[GLFW_MOUSE_BUTTON_LEFT],
		CurrentMouse[GLFW_MOUSE_BUTTON_RIGHT]
	};
}

// maps for key bindings look up and deletion
// Key combo -> InputAction lookup used by keyboardCallBack.
std::unordered_map<KeyCombo, InputAction, KeyComboHash> KeyBindings;
std::unordered_map<InputAction, KeyCombo> ActionToKey;

// used for "turning on" rebind mode
static bool WaitingForRebind = false;

// used for ui left panel print if attempted keyCombo didn't work
static bool RebindFailed = false;

// InputAction attempting to rebind
static InputAction RebindTarget;

// Binds the mouse callback functions
void InputManager::bindMouseCallbacks(
	MouseMoveCallback moveCb,
	MouseButtonCallback buttonCb,
	MouseScrollCallback scrollCb)
{
	// Store AppController handlers for later dispatch from GLFW callbacks.
	mouseMoveCallback = std::move(moveCb);
	mouseButtonCallback = std::move(buttonCb);
	mouseScrollCallback = std::move(scrollCb);
}

void InputManager::bindInputActionCallback(InputActionCallback actionCb)
{
	// Store AppController keyboard handler for InputAction dispatch.
	inputActionCallback = std::move(actionCb);
}

// set the glfw callback functions up
void InputManager::init(GLFWwindow* window)
{
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetKeyCallback(window, keyboardCallBack);
	bindDefaultKeybinds();
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
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (action == GLFW_PRESS) {
		CurrentMouse[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		CurrentMouse[button] = false;
	}

	// Forward the raw button event and current mouse snapshot to AppController.
	if (mouseButtonCallback) {
		mouseButtonCallback(buildMouseState(), button, action, mods);
	}
}

// cursor x,y position callback, when left mouse button is pressed based on the mode, does the respective thing
// also consistenly updates the currX/currY with new x,y corrdinates when mouse moves
void InputManager::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	lastX = currX;
	lastY = currY;
	deltaX = xpos - currX;
	deltaY = ypos - currY;

	currX = xpos;
	currY = ypos;

	// Forward move events continuously so controller can process drag behavior.
	if (mouseMoveCallback) {
		mouseMoveCallback(buildMouseState());
	}
}

// scoll wheel call back, used for mouse wheel scrolling and rotating when holding r key
void InputManager::scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	// Forward wheel input with current pointer position for zoom-around-cursor logic.
	if (mouseScrollCallback) {
		mouseScrollCallback(buildMouseState(), xoffset, yoffset);
	}

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

		if (temp != KeyBindings.end() && inputActionCallback)
		{
			inputActionCallback(temp->second);
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

	// Store combo -> action mapping; AppController decides what each action does.
	KeyBindings[combo] = action;

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

void InputManager::shutdown()
{
	saveKeybinds();
}

void InputManager::saveKeybinds()
{
    json j;

    for (const auto& [action, combo] : ActionToKey)
    {
		std::string str;
		if (action == InputAction::setRotate) str = "setRotate";
		if (action == InputAction::setPan) str = "setPan";
		if (action == InputAction::setDraw) str = "setDraw";
		if (action == InputAction::setErase) str = "setErase";
		if (action == InputAction::setFill) str = "setFill";
		if (action == InputAction::undo) str = "undo";
		if (action == InputAction::redo) str = "redo";
		if (action == InputAction::resetView) str = "resetView";
		if (action == InputAction::setColor) str = "setColor";
		if (action == InputAction::setClickZoomIn) str = "zoomIn";
		if (action == InputAction::setClickZoomOut) str = "zoomOut";
		if (action == InputAction::onionSkinToggle) str = "onionSkinToggle";
		if (action == InputAction::nextFrame) str = "nextFrame";
		if (action == InputAction::prevFrame) str = "prevFrame";
		if (action == InputAction::newFile) str = "newFile";
		if (action == InputAction::newFrame) str = "newFrame";
		if (action == InputAction::closeCanvas) str = "closeCanvas";

        j[str] = {
            {"key", combo.key},
            {"mods", combo.mods}
        };
    }

    std::ofstream file("assets/keybinds.json");
    if (file.is_open())
    {
        file << j.dump(4);
    }
}

bool InputManager::loadKeybinds()
{
    std::ifstream file("assets/keybinds.json");

    if (!file.is_open())
    {
        return false;
    }

    json j;
    file >> j;

    for (auto& [actionStr, value] : j.items())
    {
        InputAction action;
		if (actionStr == "setRotate") action = InputAction::setRotate;
		if (actionStr == "setPan") action = InputAction::setPan;
		if (actionStr == "setDraw") action = InputAction::setDraw;
		if (actionStr == "setErase") action = InputAction::setErase;
		if (actionStr == "setFill") action = InputAction::setFill;
		if (actionStr == "undo") action = InputAction::undo;
		if (actionStr == "redo") action = InputAction::redo;
		if (actionStr == "resetView") action = InputAction::resetView;
		if (actionStr == "setColor") action = InputAction::setColor;
		if (actionStr == "zoomIn") action = InputAction::setClickZoomIn;
		if (actionStr == "zoomOut") action = InputAction::setClickZoomOut;
		if (actionStr == "onionSkinToggle") action = InputAction::onionSkinToggle;
		if (actionStr == "nextFrame") action = InputAction::nextFrame;
		if (actionStr == "prevFrame") action = InputAction::prevFrame;
		if (actionStr == "newFile") action = InputAction::newFile;
		if (actionStr == "newFrame") action = InputAction::newFrame;
		if (actionStr == "closeCanvas") action = InputAction::closeCanvas;

        int key = value["key"];
        int mods = value["mods"];

        // bind the action
        bindAction(action, key, mods);
    }

    return true;
}

void InputManager::bindDefaultKeybinds()
{
	bindAction(InputAction::setRotate, GLFW_KEY_R, 0);
	bindAction(InputAction::setPan, GLFW_KEY_H, 0);
	bindAction(InputAction::setDraw, GLFW_KEY_D, 0);
	bindAction(InputAction::setFill, GLFW_KEY_F, 0);
	bindAction(InputAction::setErase, GLFW_KEY_E, 0);
	bindAction(InputAction::undo, GLFW_KEY_Z, GLFW_MOD_CONTROL);
	bindAction(InputAction::redo, GLFW_KEY_X, GLFW_MOD_CONTROL);
	bindAction(InputAction::resetView, GLFW_KEY_R, GLFW_MOD_CONTROL);
	bindAction(InputAction::setColor, GLFW_KEY_C, 0);
	bindAction(InputAction::setClickZoomIn, GLFW_KEY_Z, 0);
	bindAction(InputAction::setClickZoomOut, GLFW_KEY_Z, GLFW_MOD_SHIFT);
	bindAction(InputAction::onionSkinToggle, GLFW_KEY_4, 0);
	bindAction(InputAction::nextFrame, GLFW_KEY_2, 0);
	bindAction(InputAction::prevFrame, GLFW_KEY_1, 0);
	bindAction(InputAction::newFile, GLFW_KEY_N, GLFW_MOD_CONTROL);
	bindAction(InputAction::newFrame, GLFW_KEY_3, 0);
	bindAction(InputAction::closeCanvas, GLFW_KEY_W, GLFW_MOD_CONTROL);
}