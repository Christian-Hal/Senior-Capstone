
#pragma once

#include <glfw/glfw3.h>
#include <string>
#include <functional>

class Renderer;
class CanvasManager;
class Globals;

// Snapshot of mouse information forwarded to AppController.
struct MouseState {
    double x;
    double y;
    double dx;
    double dy;
    bool leftDown;
    bool rightDown;
};

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
    setFill,
    undo,
    redo,
    resetView,
    setColor,
    setClickZoomIn,
    setClickZoomOut,
    onionSkinToggle,
    nextFrame,
    prevFrame,
    newFile,
    newFrame,
    removeFrame,
    quickPlay,
    closeCanvas,
    saveFile,
    openFile
};

class InputManager
{
public:
    static void init(GLFWwindow* window);
    void shutdown();
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

    // InputManager captures raw GLFW events, then forwards them through callback functions
    // AppController binds the functions and decides behavior using AppState's info
    using MouseMoveCallback = std::function<void(const MouseState&)>;
    using MouseButtonCallback = std::function<void(const MouseState&, int button, int action, int mods)>;
    using MouseScrollCallback = std::function<void(const MouseState&, double xoffset, double yoffset)>;
    using InputActionCallback = std::function<void(InputAction)>;

    // Setup callback functions for the mouse move/button/scroll events.
    static void bindMouseCallbacks(
        MouseMoveCallback moveCb,
        MouseButtonCallback buttonCb,
        MouseScrollCallback scrollCb
    );

    // Setup callback function for resolved keyboard actions.
    static void bindInputActionCallback(InputActionCallback actionCb);

private:
    static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
    static void keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

    // functions to enable the saving and loading of the keybinds
    static void saveKeybinds();
    static bool loadKeybinds(); // returns true if keybinds were successfully loaded
    static void bindDefaultKeybinds();

};
