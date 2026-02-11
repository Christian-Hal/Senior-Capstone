
#include "DrawSystem.h"

#include <iostream>

void DrawSystem::init()
{
    strokeManager.init();
}

void DrawSystem::clear()
{
    strokeManager.clearStrokes();
}

void DrawSystem::addPointToStroke(int x, int y)
{
    strokeManager.addPoint(x, y);
}

void DrawSystem::process()
{
    if (strokeManager.hasStrokes()) {strokeManager.process();}
}