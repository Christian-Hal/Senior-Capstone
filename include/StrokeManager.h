#pragma once

#include <glm/glm.hpp>
#include <vector>

class StrokeManager
{
    public:
        bool drawing = false;

        void init();

        // Just empties the current stroke list
        void beginStroke();
        void endStroke();

        // Pushes a point to the back of the current stroke list
        void addPoint(glm::vec2 point);

        // Processes the current stroke list and generates a smoothed event path, which is stored in curEventPath
        // it then returns the smoothed event path as a list of points
        std::vector<glm::vec2> process();

        // Returns true if there are still points in the current stroke list
        bool hasValues();

    private:
        // A list of points that make up the current stroke
        std::vector<glm::vec2> currentStroke;

        // Smooths and returns the given point
        glm::vec2 smoothPoint(glm::vec2 point);

        // The last point processed by the smoother, used for calculating the smoothed point
        glm::vec2 lastSmoothed;
        bool isSmoothing = false;

        // Keeps track of the current smoothed stroke path
        // this will be used for undo and what not, holds the entire stroke from mouse down to mouse up
        std::vector<glm::vec2> totalEventPath;

};