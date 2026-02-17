
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
}

void DrawEngine::update()
{
    if (strokeManager.hasValues()) {
        // Get the smoothed event path from the stroke manager
        std::list<glm::vec2> eventPath = strokeManager.process();

        // Draw the smoothed point event path
        drawPath(eventPath);

        // TODO : brush dabs and spacing and all that stuff
        // generate the brush dab
        // move along the path while stamping the brush dab
    }
}

void DrawEngine::drawPath(const std::list<glm::vec2>& eventPath)
{
    // this is straight up just a copy paste of the old drawing code with some very tiny modifications
    // I made it run over a set of positions instead of just one like the old code
    // I plan on changing this a lot and making it better and having actual comments but for now I just wanted something working

    Canvas& curCanvas = canvasManager.getActive();
    float x, y;

    for (const auto& point : eventPath)
    {
            x = point.x;
            y = point.y;

        if (!hasLastPos)
        {
            lastX = x;
            lastY = y;
            lastDrawnX = x;
            lastDrawnY = y;
            hasLastPos = true;
            curCanvas.setPixel(x, y, ui.getColor());
            continue;
        }

        float dx = x - lastX;
        float dy = y - lastY;

        int steps = (int)std::ceil(std::max(std::abs(dx), std::abs(dy)));

        // logic to prevent system crashing if zoomed in too much
        if (steps == 0)
        {
            // Draw a single dab and bail out
            curCanvas.setPixel(x, y, ui.getColor());
            lastX = x;
            lastY = y;
            continue;
        }

        // grab and compute the brush info
        if (brushManager.brushChange == true)
        {
            activeBrush = brushManager.getActiveBrush();
            brushManager.brushChange = false;
        }

        int size = ui.brushSize;
        int w = activeBrush.tipWidth;
        int h = activeBrush.tipHeight;
        int brushSpacing = size * activeBrush.spacing;
        std::vector<float> alpha = activeBrush.tipAlpha;

        int brushCenter_x = w / 2;
        int brushCenter_y = h / 2;

        float invSteps = 1.0f / (float)steps;

        // for each step between the last position and current position
        for (int i = 0; i <= steps; i++)
        {
            // had to change some math becuase it was crashing if trying to draw too zoomed in
            int baseX = lastX + (int)(dx * i * invSteps) - brushCenter_x * size;
            int baseY = lastY + (int)(dy * i * invSteps) - brushCenter_y * size;

            float distance = sqrt(((lastDrawnX - baseX) * (lastDrawnX - baseX)) +  ((lastDrawnY - baseY) * (lastDrawnY - baseY)));
            if (distance < brushSpacing)
                continue;

            bool stamped = false;

            // for each row in the brush mask
            for (int r = 0; r < h; r++)
            {
                // for each column in the brush mask
                for (int c = 0; c < w; c++)
                {
                    // if the current index is part of the pattern
                    float a = alpha[r * w + c];
                    if (a > 0.01f) 
                    {
                        for (int sy = 0; sy < size; sy++)
                        {
                            for (int sx = 0; sx < size; sx++)
                            {
                                // calculate the pixel x and y on the canvas
                                int px = baseX + c * size + sx;
                                int py = baseY + r * size + sy;
                                
                                curCanvas.setPixel(px, py, ui.getColor());
                            }
                        }

                        stamped = true;
                    }
                }
            }
            if (stamped)
            {
                lastDrawnX = baseX;
                lastDrawnY = baseY;
            }
        }

        lastX = x;
        lastY = y;
    }
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