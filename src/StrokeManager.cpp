
#include "StrokeManager.h"

#include <iostream>

void StrokeManager::init()
{
    currentStroke = {};
}

void StrokeManager::endStroke()
{
    // add current stroke into list of recent actions (for UNDO)

    // clear the current stroke
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
    // Grab first element of the list
    std::pair<int, int> point = currentStroke.front();
    currentStroke.remove(point);

    // Process it: for now just a debug print
    std::cout << "(" << point.first << ", " << point.second << ")\n";

    // need to smooth between points
    // run through the current stroke list and smooth each point into a better line
    // not sure how to do this right now
}