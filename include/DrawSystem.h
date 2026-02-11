#pragma once

#include "StrokeManager.h"

#include <list>

class DrawSystem
{
    public:
        bool drawing = false;

        void init();

        // Tells the stroke manager to stop the current stroke list
        void stop();

        // Tells the stroke manager to add a point to the current stroke list
        void addPointToStroke(int x, int y);

        // Processes the current draw state
        void process();

    private:
        StrokeManager strokeManager;

};