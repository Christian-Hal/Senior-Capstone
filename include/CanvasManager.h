
#pragma once

#include "Canvas.h"

#include <vector>
#include <string>

class CanvasManager {
    public:
        Canvas& createCanvas(int width, int height, std::string name);
        Canvas& getActive();
        bool hasActive();

        int getNumCanvases();
        const std::vector<Canvas>& getOpenCanvases() const;
        void setActiveCanvas(int index);

        // this is set to true when the active canvas is changed and
        // set to false when something acknolwedges the change
        bool canvasChange = false;

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
        


    private:
        // list of active canvases for when we implement the tab system
        std::vector<Canvas> canvases;

        // pointer to the active canvas
        Canvas* activeCanvas = nullptr;
        int activeIndex = -1;

        // just a helper function to avoid having the same name in multiple files
        std::string checkName(std::string name);
};