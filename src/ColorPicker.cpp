
#include "ColorPicker.h"
#include "DrawEngine.h"
#include "UI.h"


extern UI ui;
extern CanvasManager activeCanvasManager;
extern DrawEngine drawEngine;

void ColorPicker::pickColor(double mouseX, double mouseY)
{
	if (activeCanvasManager.hasActive())
	{
		Canvas& canvas = activeCanvasManager.getActive();

		glm::vec2 canvasCoords = drawEngine.mouseToCanvasCoords(mouseX, mouseY);
		int canvasX = static_cast<int>(canvasCoords.x);
		int canvasY = static_cast<int>(canvasCoords.y);

		if (canvasX >= 0 && canvasX < canvas.getWidth() && canvasY >= 0 && canvasY < canvas.getHeight())
		{
			Color pickedColor = canvas.getPixel(canvasX, canvasY);
			ui.setColor(pickedColor);
		}
	}
}