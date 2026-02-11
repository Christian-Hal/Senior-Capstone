
#include "StrokeManager.h"

#include <iostream>

void StrokeManager::init()
{
    currentStroke = {};
}

void StrokeManager::clearStrokes()
{
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
    // Pop the first element of the list
    std::pair<int, int> point = currentStroke.front();
    currentStroke.pop_front();

    // Process it: for now just a debug print
    std::cout << "(" << point.first << ", " << point.second << ")\n";
}