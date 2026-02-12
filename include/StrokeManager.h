#pragma once

#include <list>

class StrokeManager
{
    public:
        bool drawing = false;

        void init();

        // Just empties the current stroke list
        void endStroke();

        // Pushes a point to the back of the current stroke list
        void addPoint(int x, int y);

        // Pops and processes the first point in the current stroke list
        void process();

        // Returns true if there are still points in the current stroke list
        bool hasStrokes();

    private:
        // A list of points that make up the current stroke
        std::list<std::pair<int,int>> currentStroke;

        std::pair<int, int> pointSmoother();

};