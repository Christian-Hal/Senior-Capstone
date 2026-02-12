
#include "StrokeManager.h"
#include "CanvasManager.h"

#include <iostream>
#include <list>

static CanvasManager activeCanvasManager;

void StrokeManager::init()
{
    currentStroke = {};
}

void StrokeManager::endStroke()
{
    // Process any remaining points in the stroke
    if (hasStrokes()) {process();}

    // TODO : Add final event path into list of recent actions (for UNDO)

    // Clear the current stroke
    currentStroke.clear();
}

void StrokeManager::addPoint(int x, int y)
{
    currentStroke.push_back({x, y});
}

bool StrokeManager::hasStrokes()
{
    return !currentStroke.empty();
}

void StrokeManager::process()
{
    std::cout << "processing!" << std::endl;
    while(!currentStroke.empty()) {
        // Pop the first element out of the list
        std::pair<int, int> point = currentStroke.front();
        currentStroke.remove(point);

        // TODO : Implement smoothing and interpolation
        // Smooth the points

        // Interpolate the points
    }

    // TODO : Implement event path
    // Add the fixed segment to the curEventPath

    // TODO : update function to return the smoothed path
    // return path
}