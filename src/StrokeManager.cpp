
#include "StrokeManager.h"
#include "CanvasManager.h"

#include <iostream>
#include <list>

static CanvasManager activeCanvasManager;

void StrokeManager::init()
{
    currentStroke = {};
    curEventPath = {};
}

void StrokeManager::endStroke()
{
    // Process any remaining points in the stroke
    if (hasValues()) {process();}

    // TODO : Add final event path into list of recent actions (for UNDO)

    // Clear the current stroke and event path
    currentStroke.clear();
    curEventPath.clear();
    isSmoothing = false;
}

void StrokeManager::addPoint(std::pair<float, float> point)
{
    currentStroke.push_back(point);
}

bool StrokeManager::hasValues()
{
    return !currentStroke.empty();
}

std::list<std::pair<float, float>> StrokeManager::process()
{
    std::list<std::pair<float, float>> eventPath;

    while(!currentStroke.empty()) {
        // Pop the first element out of the list
        std::pair<float, float> point = currentStroke.front();
        currentStroke.pop_front();
        std::cout << "Processing point: (" << point.first << ", " << point.second << ")" << std::endl;

        // Smooth the point
        std::pair<float, float> smoothedPoint = smoothPoint(point);
        eventPath.push_back(smoothedPoint);
        curEventPath.push_back(smoothedPoint);
    }

    return eventPath;
}

std::pair<float, float> StrokeManager::smoothPoint(std::pair<float, float> point)
{
    // if it isn't smoothing then start smoothing
    // set the last smoothed point to the current point and return the current point as the smoothed point
    if (!isSmoothing) {
            lastSmoothed = point;
            isSmoothing = true;
            return point;
    }
    
    float alpha  = 0.25f;
    float invAlpha = 1.0f - alpha;

    // implementation of a simple exponential moving average smoother
    // formula : last = raw * alpha + last * (1.0f - alpha)
    std::pair<float, float> p1 = {alpha * point.first, alpha * point.second};
    std::pair<float, float> p2 = {invAlpha * lastSmoothed.first, invAlpha * lastSmoothed.second};

    // combine the first two halves of the formula to get the smoothed point
    lastSmoothed = {p1.first + p2.first, p1.second + p2.second};
    return lastSmoothed;
}