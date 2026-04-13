
#include "Canvas.h"
#include "DrawEngine.h"

#include <algorithm>
#include <iostream>

void DrawEngine::init()
{
    // set some default values
    drawing = false;
    spacing = 1;
    hasPrev = false;
    drawSize = 1;
    distanceSinceLastStamp = 0;
    curCanvas = nullptr;

    // initialize the stroke manager
    strokeManager.init();
}


bool DrawEngine::isDrawing()
{
    return drawing;
}

void DrawEngine::start()
{
    // if missing the information needed to draw then don't draw
    if (!curCanvas || curBrushDab.size() < 2) {
        return;
    }

    //std::cout << "Draw engine start!" << std::endl;
    drawing = true;
    didStamp = false;

    // start the stroke
    strokeManager.beginStroke();
    curCanvas->beginStrokeRecord();
}

void DrawEngine::stop()
{
    //std::cout << "Draw engine stop!" << std::endl;
    drawing = false;
    strokeManager.endStroke();

    if (!didStamp && hasPrev) stampBrush(prev);

    hasPrev = false;
    distanceSinceLastStamp = 0;
    if (curCanvas) {
        curCanvas->endStrokeRecord();
        curCanvas->isDirty = true;
    }
}



void DrawEngine::update()
{
    if (strokeManager.hasValues()) {
        // Get the smoothed event path from the stroke manager
        std::vector<glm::vec2> eventPath = strokeManager.process();

        // Draw the smoothed point event path
        drawPath(eventPath);
    }
}

void DrawEngine::drawPath(const std::vector<glm::vec2>& eventPath)
{
    const float brushDiameter = (curBrushDab[0] + curBrushDab[1]) / 2.0f;
    const float stampInterval = std::max(0.001f, spacing * brushDiameter);

    // for each smoothed point in the event path
    for (const auto& point : eventPath)
    {
        // if there isn't a previous point then we can't draw anything yet
        if (!hasPrev) 
        {
            // set up prev for the next point and continue through the loop
            prev = point;
            hasPrev = true;
            continue;
        }

        // get the vector between the two points and its length
        glm::vec2 delta = point - prev;
        float len = length(delta);

        // if the points are the same then don't draw anything
        if (len == 0) continue;

        // normalize the delta to get the direction of the stroke
        glm::vec2 dir = delta / len;

        // keeps track of how much distance is left
        float remaining = len;

        // this basically checks if theres enough space left to stamp another brush dab
        while (distanceSinceLastStamp + remaining >= stampInterval) 
        {
            // how far we need to step to get to the next position
            float step = stampInterval - distanceSinceLastStamp;

            // calculate the position of the next stamp and stamp
            glm::vec2 stampPos = prev + dir * step;
            stampBrush(stampPos);
            didStamp = true;

            // update the remaining distance and prev for the next loop iteration
            prev = stampPos;
            remaining -= step;
            distanceSinceLastStamp = 0;
        }

        // carry any left over distance to the next point
        distanceSinceLastStamp += remaining;
        prev = point;
    }
}

void DrawEngine::stampBrush(glm::vec2 position)
{
    if (!curCanvas || curBrushDab.size() < 2) {
        return;
    }

    int W = static_cast<int>(std::lround(curBrushDab[0])); 
    int H = static_cast<int>(std::lround(curBrushDab[1])); 

    // this copies everything but the first two values, which are the width and height
    std::vector<float> alpha(curBrushDab.begin() + 2, curBrushDab.end());

    // calculate other needed information
    int topLeftX = static_cast<int>(std::floor(position.x - (W / 2.0f))); 
    int topLeftY = static_cast<int>(std::floor(position.y - (H / 2.0f))); 

    for (int r = 0; r < H; r++) 
    {
        for (int c = 0; c < W; c++) 
        {
            float a = alpha[r * W + c];
            if (a > 0.01f) {
                int finalX = topLeftX + c;
                int finalY = topLeftY + r;

                curCanvas->blendPixel(finalX, finalY, curColor, curColor.a/255.0);
            }
        }
    }
}



// takes in a set of canvas coords and adds them as a point in the current stroke
void DrawEngine::processMousePos(double canvasX, double canvasY)
{
	// add the point into the stroke path
	strokeManager.addPoint(glm::vec2(canvasX, canvasY));
}

// updates the information needed to draw: the canvas being drawn on, the brush dab being stamped, and the color used to draw
void DrawEngine::setBrushDab(std::vector<float> newBrushDab, float spacing, int drawSize) {
    //curBrushDab = newBrushDab;
    curBrushDab = std::move(newBrushDab); 

    this->spacing = spacing;
    //this->drawSize = drawSize;
    this->drawSize = std::max(1, drawSize); 
}
void DrawEngine::setColor(Color newColor) {
    curColor = newColor;
}
void DrawEngine::setCanvas(Canvas& newCanvas) {
    curCanvas = &newCanvas;
}
