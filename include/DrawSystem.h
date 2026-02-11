#pragma once

#include "StrokeManager.h"

#include <list>

class DrawSystem
{
    public:
        bool drawing = false;

        void init();

        void clear();

        void addPointToStroke(int x, int y);

        void process();

    private:
        StrokeManager strokeManager;

};