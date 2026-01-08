
#include "CanvasManager.h"
#include <vector>

Canvas& CanvasManager::createCanvas(int width, int height)
{
    canvases.emplace_back(Canvas(width, height));
    active = &canvases.back();

    canvasChange = true;

    return *active;
}

Canvas& CanvasManager::getActive()
{
    return *active;
}

bool CanvasManager::hasActive()
{
    return active != nullptr;
}