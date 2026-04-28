
#pragma once

#include "Canvas.h"
#include "imgui.h"

#include <vector>
#include <string>

class CanvasManager {
    public:
        Canvas& createCanvas(int width, int height, std::string name, bool isAnimation, bool useAnimTemplate, 
            const ImVec4& paperColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        Canvas& getActive();
        bool hasActive();

        void closeCanvas(int index);
        int getActiveCanvasIndex() const;

        int getNumCanvases();
        const std::vector<Canvas>& getOpenCanvases() const;
        void setActiveCanvas(int index);

        // this is set to true when the active canvas is changed and
        // set to false when something acknolwedges the change
        bool canvasChange = false;
        bool paperChange = false;

        // saving 
        void getFrameData(CanvasManager& canvasManagerA);

        // add completed stroke to canvas stroke history for undo/redo
        void addStroke(std::vector<glm::vec2>& strokePath);

        // draw to the active canvas
        void draw(int x, int y, const Color& color);

        // for undo / redo commands
        // they just tell the active canvas to run an undo or redo
        void undo();
        void redo();

        // commands for grabbing and edditing canvas info
        ImVec4 getPaperColor();
        void setPaperColor(const ImVec4& color);

        void saveToFile(const std::string& path);
        void loadFromFile(const std::string& filepath);
        void saveORA(const std::string& path);
        void loadORA(const std::string& filepath);


        // index of the active canvas
        int activeCanvasIndex = -1;
    private:
        // list of active canvases for when we implement the tab system
        std::vector<Canvas> canvases;

        

        // just a helper function to avoid having the same name in multiple files
        std::string checkName(std::string name);
};