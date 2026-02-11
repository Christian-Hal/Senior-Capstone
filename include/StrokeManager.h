#pragma once

#include <list>

class StrokeManager
{
    public:
        bool drawing = false;

        void init();

        void clearStrokes();

        void addPoint(int x, int y);

        void process();

        bool hasStrokes();

    private:
        std::list<std::pair<int,int>> currentStroke;

};