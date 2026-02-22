
#pragma once

#include <glfw/glfw3.h>
#include "UI.h"
class Renderer;
class CanvasManager;

class Globals;

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

    static bool bindKey(UI::CursorMode mode, int key);

    static void StartRebind(UI::CursorMode mode);
    static bool IsWaitingForRebind();
    static bool getRebindFail();


private:
    static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
    static void keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

};
