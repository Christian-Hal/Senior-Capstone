
#include "DrawSystem.h"

#include <iostream>

void DrawSystem::init()
{
    strokeManager.init();
}

void DrawSystem::stop()
{
    strokeManager.endStroke();
}

void DrawSystem::addPointToStroke(int x, int y)
{
    strokeManager.addPoint(x, y);
}

void DrawSystem::process()
{
    if (strokeManager.hasStrokes()) {strokeManager.process();}
}