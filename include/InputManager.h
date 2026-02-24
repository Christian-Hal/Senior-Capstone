
#pragma once

#include <glfw/glfw3.h>
#include "UI.h"
class Renderer;
class CanvasManager;
class Globals;




struct KeyCombo
{
    int key;
    int mods; // GLFW_MOD_CTRL, GLFW_MOD_ALT, GLFW_MOD_SHIFT, etc.

    // this allows for only comparisons that only return true is both the key and modifyer are the same
    bool operator==(const KeyCombo& other) const
    {
        return key == other.key && mods == other.mods;
    }
};

// hash is used for fast lookup, when we add more hotkeys to various things instead of looking through every keyCombo in the unordered map
// the has lets for dictionary lookup
// the hash is created by hashing both the key and the mods, shifting the mods hash one to the left, then combining them with XOR
struct KeyComboHash
{
    std::size_t operator()(const KeyCombo& k) const
    {
        return std::hash<int>()(k.key) ^ (std::hash<int>()(k.mods) << 1);
    }
};

enum class InputAction
{
    setRotate,
    setPan,
    setDraw,
    setErase,
    undo,
    redo,
    resetView
    //setZoomDragging
};


class InputManager
{
public:
    static void init(GLFWwindow* window, Renderer* renderer);
    static void update();
    static bool IsMousePressed(int button);
    static double getMouseX();
    static double getMouseY();
    static double getMouseDeltaX();
    static double getMouseDeltaY();
    static bool bindAction(InputAction action, int key, int mods);
    static void StartRebind(InputAction action);
    static bool IsWaitingForRebind();
    static bool getRebindFail();
    static bool isModifierKey(int key);
    static std::string getKeybind(const KeyCombo& combo);
    static std::string getHotkeyString(InputAction action);

private:
    static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
    static void keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

};
