
#include "DrawEngine.h"
#include "UI.h"
#include "CanvasManager.h"
#include "BrushManager.h"
#include "Globals.h"

#include <iostream>

extern Globals global;
extern UI ui;
extern CanvasManager canvasManager;
extern BrushManager brushManager;
BrushTool activeBrush;

void DrawEngine::init()
{
    drawing = false;
    spacing = 1;
    hasPrev = false;
    distanceSinceLastStamp = 0;

    strokeManager.init();
}

bool DrawEngine::isDrawing()
{
    return drawing;
}

void DrawEngine::start()
{
    //std::cout << "Draw engine start!" << std::endl;
    drawing = true;
}

void DrawEngine::stop()
{
    //std::cout << "Draw engine stop!" << std::endl;
    drawing = false;
    hasLastPos = false;
    strokeManager.endStroke();

    hasPrev = false;
    distanceSinceLastStamp = 0;
}

void DrawEngine::update()
{
    if (strokeManager.hasValues()) {
        // Get the smoothed event path from the stroke manager
        std::list<glm::vec2> eventPath = strokeManager.process();

        // Draw the smoothed point event path
        drawPath(eventPath);
    }
}

void DrawEngine::drawPath(const std::list<glm::vec2>& eventPath)
{
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
        while (distanceSinceLastStamp + remaining >= spacing) 
        {
            // how far we need to step to get to the next position
            float step = spacing - distanceSinceLastStamp;

            // calculate the position of the next stamp and stamp
            glm::vec2 stampPos = prev + dir * step;
            stampBrush(stampPos);

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
    // this is where the code for stamping the brush dab onto the canvas will go
    Canvas& curCanvas = canvasManager.getActive();
    curCanvas.setPixel((int)position.x, (int)position.y, ui.getColor());
}

// takes in a mouse position adds the converted canvas coords as a point in the current stroke
void DrawEngine::processMousePos(double mouseX, double mouseY)
{
    // convert the moust position to canvas coordinates
	glm::vec2 point = mouseToCanvasCoords(mouseX, mouseY);

	// add the point into the stroke path
	strokeManager.addPoint(point);
}

// takes in a mouse position and returns the converted pixel coordinates on the current canvas
glm::vec2 DrawEngine::mouseToCanvasCoords(double mouseX, double mouseY)
 {
    // grab the current canvas
	Canvas& curCanvas = canvasManager.getActive();

	// convert the mouse position to screen space (with y flipped)
	float screenX = mouseX;
	float screenY = global.get_scr_height() - mouseY;

	// grab the center of the canvas
	glm::vec2 canvasCenter(
		curCanvas.getWidth() * 0.5f,
		curCanvas.getHeight() * 0.5f
	);

	// stores the point as a vector for easier manipulation(?)
	// not sure what the naming convetion is for this cause Gunter wrote this stuff
	// will probably change it later lol
	glm::vec2 p = { screenX, screenY };

	// removes the canvases offset and ensures its centered at (0,0)
	p -= curCanvas.offset;
	p -= canvasCenter;

	// calculate the cosine and sine of the negative rotation angle for unrotating the point
	float c = cosf(-curCanvas.rotation);
	float s = sinf(-curCanvas.rotation);

	// Simple rotation matrix to rotate the point
	// if the canvas is rotated X degrees then we need to rotate the point -X degrees to match the canvas space
	p = {
		p.x * c - p.y * s,
		p.x * s + p.y * c
	};

	// undo the zoom by dividing the point by the zoom level
	// if the the canvas is zoomed in by 2 then dividing by 2 will remove the zoom
	p /= curCanvas.zoom;

	// move origin back to normal coordinate space
	p += canvasCenter;

    return glm::vec2(p.x, p.y);
 }

