
#include "AppState.h"

#include <iostream>
#include <fstream>

Globals global;

void AppState::init()
{
    // --- Set up MockUp folder in user's Documents ---
    char* docPath = nullptr;

    // this sets it up for windows and mac systems
    #ifdef _WIN32
        docPath = getenv("USERPROFILE");
    #else
        docPath = getenv("HOME");
    #endif

    // save the mockup folder path and create it if needed
    if (docPath) {
        mockUpFolderPath = std::filesystem::path(docPath) / "Documents" / "MockUp";
    } else {
        mockUpFolderPath = "MockUp";
    }
    defaultFolderPath = mockUpFolderPath;
    
    // create it if needed
    if (!std::filesystem::exists(mockUpFolderPath)) {
        std::filesystem::create_directories(mockUpFolderPath);
    }

    // initialize and load recent activity list
    recentActivity = std::vector<std::string>();
    loadRecentActivity();
}

void AppState::shutdown()
{
    // shut down components
    frameRenderer.shutdown();
	ui.shutdown();
	renderer.shutdown();
	window.destroy();

    // save recent activity to file
    saveRecentActivity();
}

Window& AppState::getWindow() { return window; }
Renderer& AppState::getRenderer() { return renderer; }
UI& AppState::getUI() { return ui; }
Globals& AppState::getGlobals() { return global; }
CanvasManager& AppState::getCanvasManager() { return canvasManager; }
BrushManager& AppState::getBrushManager() { return brushManager; }
DrawEngine& AppState::getDrawEngine() { return drawEngine; }
FrameRenderer& AppState::getFrameRenderer() { return frameRenderer; }
InputManager& AppState::getInputManager() { return inputManager; }

CursorMode AppState::getCursorMode() const
{
    return currentCursorMode;
}

void AppState::setCursorMode(CursorMode mode)
{
    currentCursorMode = mode;
}

void AppState::addFileToRecentActivity(const std::string& filePath)
{
    // Add file path to the recent activity list if its not already there and if the file exists
    if ((std::find(recentActivity.begin(), recentActivity.end(), filePath) == recentActivity.end()) && std::filesystem::exists(filePath)){
        recentActivity.push_back(filePath);
    }

    // Limit the recent acvity to 10, CSP does this and it keeps it from getting super cluttered and showing really old files
    if (recentActivity.size() > 10) {
        recentActivity.erase(recentActivity.begin());
    }
}

const std::vector<std::string>& AppState::getRecentActivity()
{
    return recentActivity;
}

void AppState::loadRecentActivity()
{
    // path to the file containing the recent activity list in MockUp folder
    std::filesystem::path filePath = "assets/recent_activity.txt";

    // if the file exists
    if (std::filesystem::exists(filePath)) 
    { 
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Skip empty lines
                if (!line.empty()) {
                    addFileToRecentActivity(line);
                }
            }
            file.close();
        }
        else {
            std::cout << "Unable to open recent activity file" << std::endl;
        }
    } 
    else {
        // create the file and leave it empty
        std::ofstream file(filePath);
        if (file.is_open()) {
            file.close();
        }
    }
}

void AppState::saveRecentActivity()
{
    // path to the file containing the recent activity list in MockUp folder
    std::filesystem::path filePath = "assets/recent_activity.txt";

    // open the file and write the recent activity list to it
    std::ofstream file(filePath);
    if (file.is_open()) {
        for (const std::string& filePath : recentActivity) {
            file << filePath << std::endl;
        }
        file.close();
    }
    else {
        std::cout << "Unable to open recent activity file for writing" << std::endl;
    }
}