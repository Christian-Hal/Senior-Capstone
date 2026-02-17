#pragma once

#include "StrokeManager.h"

#include <glm/glm.hpp>
#include <list>

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

        // takes in a mouse position and returns the converted pixel coordinates on the canvas
	    glm::vec2 mouseToCanvasCoords(double mouseX, double mouseY);

        // Processes the current stroke list, generate a smoothed event path, and draw said path onto the active canvas
        void update();

    private:
        StrokeManager strokeManager;
        bool drawing = false;

        // variables that store stuff fror the drawing code in drawPath
        float lastX, lastY;
        float lastDrawnX, lastDrawnY;
        bool hasLastPos = false;

        // Draw the given event path (list of points) onto the canvas
        void drawPath(const std::list<glm::vec2>& eventPath);

};