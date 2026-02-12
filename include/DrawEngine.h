#pragma once

#include "StrokeManager.h"

#include <list>

class DrawEngine
{
    public:
        void init();

        void start();

        bool isDrawing();

        // Tells the stroke manager to stop the current stroke list
        void stop();

        // Tells the stroke manager to add a point to the current stroke list
        void processMouseInput(int x, int y);

        // Processes the current draw state
        void process();

    private:
        StrokeManager strokeManager;
        bool drawing = false;

};