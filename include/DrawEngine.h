#pragma once

#include "StrokeManager.h"
#include "Canvas.h"

#include <glm/glm.hpp>
#include <vector>

class DrawEngine
{
    public:
        // intializes the draw engine and stroke manager
        void init();

        // tells the draw engine to start a new stroke
        void start();

        // returns the state of 'drawing' either true (drawing) or false (not drawing)
        bool isDrawing();

        // Tells the stroke manager to stop the current stroke list
        void stop();

        // takes in a mouse position and adds the converted canvas coord as a point in the current stroke
        void processMousePos(double mouseX, double mouseY);

        // Processes the current stroke list, generate a smoothed event path, and draw said path onto the active canvas
        void update();

        bool didStamp = false;

        // functions for updating the current brush dab, color, and canvas for drawing
        void setBrushDab(std::vector<float> newBrushDab, float spacing, int drawSize);
        void setColor(Color newColor);
        void setCanvas(Canvas& newCanvas);

    private:
        StrokeManager strokeManager;
        bool drawing = false;
        int drawSize = 1;

        // variables that store stuff for the drawing code in drawPath
        glm::vec2 prev;                         // last point we moved from
        float distanceSinceLastStamp;           // leftover distance since last stamp
        float spacing;                          // spacing between stamps
        float brushDiameter;
        float stampInterval;
        bool hasPrev;
        
        Canvas* curCanvas = nullptr;            // the canvas we're currently drawing on
        Color curColor;                         // the color we're currently drawing with
        std::vector<float> curBrushDab;         // the current brush dab being stamped

        // Draw the given event path (list of points) onto the canvas
        void drawPath(const std::vector<glm::vec2>& eventPath);
        void stampBrush(glm::vec2 position);

};