#pragma once

#include "AppState.h"
#include <string>
#include "glm/glm.hpp"

class AppController {
public:
    bool init();        // initialize the app controller and the app state
    void run();         // run the main loop of the application
    void shutdown();    // clean up resources and shut down the application

    // -- Callback methods used for interaction with the app state from components -- //
    // cursor mode
    void setCursorMode(CursorMode mode);
    CursorMode getCursorMode() const;

    // brush engine actions
    void importBrush(const std::string& path);
    void deleteBrush(int index);
    const std::vector<BrushTool>& getBrushList();
    const BrushTool& getActiveBrush();
    void setActiveBrush(int index);

    // hotkey actions
    std::string getHotkeyString(InputAction action);
    void startRebind(InputAction action);
    bool isWaitingForRebind();
    bool didRebindFail();

    // recent activity functions
    void addFileToRecentActivity(const std::string& filePath);
    const std::vector<std::string>& getRecentActivity();

private:
    // app state instance
    AppState appState; 

    // mouse callback functions
    void onMouseMove(const MouseState& m);
    void onMouseButton(const MouseState& m, int button, int action, int mods);
    void onMouseScroll(const MouseState& m, double xoffset, double yoffset);
    void onInputAction(InputAction action);

    // takes in a mouse position and returns the converted pixel coordinates on the canvas
	glm::vec2 mouseToCanvasCoords(double mouseX, double mouseY);

    // takes in a canvas and mouse position and updates the current color in the UI to the color of the pixel at that position on the canvas
    void pickColor(Canvas& canvas, double xPos, double yPos);

    // fills in the canvas with the current color in all directions until there is nowhere else to go
    void fill(Canvas& canvas, Color newColor, Color oldColor, int canvasX, int canvasY);


};