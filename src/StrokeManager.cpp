
#include "StrokeManager.h"
#include "CanvasManager.h"

#include <iostream>
#include <list>

static CanvasManager activeCanvasManager;

void StrokeManager::init()
{
    currentStroke = {};
    totalEventPath = {};
}

void StrokeManager::beginStroke()
{
    // Clear the current stroke and event path
    currentStroke.clear();
    totalEventPath.clear();
}

void StrokeManager::endStroke()
{
    // Process any remaining points in the stroke
    if (hasValues()) {process();}

    // TODO : Add final event path into list of recent actions (for UNDO)
    std::vector<glm::vec2> action = totalEventPath;

    // Clear the current stroke and event path
    currentStroke.clear();
    totalEventPath.clear();
    isSmoothing = false;

    //return action;
}

void StrokeManager::addPoint(glm::vec2 point)
{
    currentStroke.push_back(point);
}

bool StrokeManager::hasValues()
{
    return !currentStroke.empty();
}

std::vector<glm::vec2> StrokeManager::process()
{
    std::vector<glm::vec2> eventPath;

    while(!currentStroke.empty()) {
        // Grab and remove the first element of the list
        glm::vec2 point = currentStroke.front();
        currentStroke.erase(currentStroke.begin());
        //std::cout << "Processing point: (" << point.x << ", " << point.y << ")" << std::endl;

        // Smooth the point
        glm::vec2 smoothedPoint = smoothPoint(point);

        // add it into the event path for this frame
        eventPath.push_back(smoothedPoint);

        // add it into the current event path for the entire stroke
        // this will be used for undo and what not, holds the entire stroke from mouse down to mouse up
        totalEventPath.push_back(smoothedPoint);
    }

    return eventPath;
}

glm::vec2 StrokeManager::smoothPoint(glm::vec2 point)
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
    glm::vec2 p1 = {alpha * point.x, alpha * point.y};
    glm::vec2 p2 = {invAlpha * lastSmoothed.x, invAlpha * lastSmoothed.y};

    // combine the first two halves of the formula to get the smoothed point
    lastSmoothed = {p1.x + p2.x, p1.y + p2.y};
    return lastSmoothed;
}