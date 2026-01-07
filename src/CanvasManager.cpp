
#include "CanvasManager.h"
#include <vector>

Canvas& CanvasManager::createCanvas(int width, int height)
{
    canvases.emplace_back(width, height);
    active = &canvases.back();
    return *active;
}

Canvas& CanvasManager::getActive()
{
    return *active;
}