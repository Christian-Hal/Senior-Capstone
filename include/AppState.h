#pragma once

#include "Renderer.h"
#include "UI.h"
#include "Globals.h"        // need to remove this
#include "CanvasManager.h"
#include "BrushManager.h"
#include "DrawEngine.h"
#include "FrameRenderer.h"

#include "InputManager.h"   // should these two be part of controller?
#include "Window.h"
#include "CursorMode.h"

#include <filesystem>

class AppState {
public:
    // initialize the app state and create instances for the different app components
    void init();
    void shutdown();

    // central component accessors used by AppController
    Window& getWindow();
    Renderer& getRenderer();
    UI& getUI();
    Globals& getGlobals();
    CanvasManager& getCanvasManager();
    BrushManager& getBrushManager();
    DrawEngine& getDrawEngine();
    FrameRenderer& getFrameRenderer();
    InputManager& getInputManager();

    // getter and setter methods for its variables
    CursorMode getCursorMode() const;
    void setCursorMode(CursorMode mode);

    // recent activity functions
    void loadRecentActivity();
    void saveRecentActivity();

    // Folder path functions
    const std::filesystem::path& getMockUpFolderPath() const { return mockUpFolderPath; }

    const std::filesystem::path& getDefaultFolderPath() const { return defaultFolderPath; }
    void setDefaultFolderPath(const std::filesystem::path& path) { defaultFolderPath = path; }

    void addFileToRecentActivity(const std::string& filePath);
    const std::vector<std::string>& getRecentActivity();

private:
    // Centralized application components.
    Window window;
    Renderer renderer;
    UI ui;
    CanvasManager canvasManager;
    BrushManager brushManager;
    DrawEngine drawEngine;
    FrameRenderer frameRenderer;
    InputManager inputManager;

    // Current cursor mode variable
    CursorMode currentCursorMode = CursorMode::Draw; // set the default mode

    // Recent activity list
    std::vector<std::string> recentActivity;

    // default folder path variables
    std::filesystem::path mockUpFolderPath;
    std::filesystem::path defaultFolderPath;

};