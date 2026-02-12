
#include "DrawEngine.h"

#include <iostream>

void DrawEngine::init()
{
    strokeManager.init();
}

bool DrawEngine::isDrawing()
{
    return drawing;
}

void DrawEngine::start()
{
    drawing = true;
}

void DrawEngine::stop()
{
    drawing = false;
    strokeManager.endStroke();
}

void DrawEngine::processMouseInput(int x, int y)
{
    // TODO : Convert mouse position into pixel coords
    // converting to pixel coordinates is currently 
    // handled by the renderer and should not be

    // Add it to the stroke path
    strokeManager.addPoint(x, y);
}

void DrawEngine::process()
{
    if (strokeManager.hasStrokes()) {
        // TODO : make this to return the smoothed event path
        strokeManager.process();

        // TODO : Implement drawing code to draw the path
        // generate the brush dab
        // move along the path while stamping the brush dab
    }
}