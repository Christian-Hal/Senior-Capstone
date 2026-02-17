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
        void addPoint(std::pair<float, float> point);

        // Processes the current draw state
        void process();

        // Draw the given event path (list of points) onto the canvas
        void drawPath(const std::list<std::pair<float, float>>& eventPath);

    private:
        StrokeManager strokeManager;
        bool drawing = false;

        // variables that store stuff fror the drawing code in drawPath
        float lastX, lastY;
        float lastDrawnX, lastDrawnY;
        bool hasLastPos = false;

};