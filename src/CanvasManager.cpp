
#include "CanvasManager.h"
#include <vector>

Canvas& CanvasManager::createCanvas(int width, int height, std::string name)
{
    std::string fixed_name = checkName(name);
    canvases.emplace_back(Canvas(width, height, fixed_name));
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

int CanvasManager::getNumCanvases()
{
    return canvases.size();
}

std::string CanvasManager::checkName(std::string name)
{
    int i = 0;

    for (Canvas canvas : canvases) 
    {
        std::string temp = canvas.getName();
        if (temp == name) { i++; }
    }

    if (i > 0) {return (name + "-" + std::to_string(i)); }
    return name;
}

const std::vector<Canvas>& CanvasManager::getOpenCanvases() const
{
    return canvases;
}

void CanvasManager::setActiveCanvas(int index)
{
    if (index > 0 && index < canvases.size()){}
    
    active = &canvases[index];
    canvasChange = true;
}

