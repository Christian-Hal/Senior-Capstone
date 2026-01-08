
#pragma once
#include "Canvas.h"
#include <vector>

class CanvasManager {
    public:
        Canvas& createCanvas(int width, int height);
        Canvas& getActive();
        bool hasActive();

        // this is set to true when the active canvas is changed and
        // set to false when something acknolwedges the change
        bool canvasChange = false;

    private:
        // list of active canvases for when we implement the tab system
        std::vector<Canvas> canvases;

        // pointer to the active canvas
        Canvas* active = nullptr;
};