
#include "InputManager.h"

#include "Renderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

/*
	---- FILE DESC ----
	Input manager includes buffers needed for all keyboard inputs 
	Takes in mouse input 

	Contains finite state machine for the current state of the cursor 
	that state can be updated by the UI buttons in UI.cpp or with 
	keyboard input handeled here and called elsewhere 
	

*/

// -- getting mouse button down -- 




// -- input loop -- 
void InputManager::readInput() {

}
/*
static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	// if no renderer    or imgui wants the mouse
	if (!activeRenderer || ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	// if we have a button 				and im gui does NOT want the mouse
	if (button == GLFW_MOUSE_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
	{
		if (action == GLFW_PRESS)
			activeRenderer->isDrawing = true;
		else if (action == GLFW_RELEASE)
		{
			activeRenderer->isDrawing = false;
			hasLastPos = false;
		}
	}

	
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	// if no renderer	or it is not drawing 		  or ImGUI wants to use the mouse		or the file is not open
	if (!activeRenderer || !activeRenderer->isDrawing || ImGui::GetIO().WantCaptureMouse || !activeCanvasManager.hasActive())
		return;


	Canvas& curCanvas = activeCanvasManager.getActive();
	
	float centerX = global.get_scr_width() * 0.5f;
	float centerY = global.get_scr_height() * 0.5f;
	float cW = curCanvas.getWidth() * 0.5;
	float cH = curCanvas.getHeight() * 0.5;

	float canvasL = centerX - cW;
	float canvasR = centerY - cH;

	float relX = xpos - canvasL;
	float relY = ypos - canvasR;

	//if (relX < 0 || relX >= curCanvas.getWidth() || relY < 0 || relY >= curCanvas.getHeight())
	//	return;
	
	//std::cout << "x,y" << xpos << ", " << ypos << std::endl;

	int x = static_cast<int>(relX);
	int y = static_cast<int>(curCanvas.getHeight() - 1 - static_cast<int>(relY));
	
	if (!hasLastPos)
	{
		lastX = x;
		lastY = y;
		hasLastPos = true;
		curCanvas.setPixel(x, y, ui.getColor());
		return;
	}

	int dx = x - lastX;
	int dy = y - lastY;
	int steps = std::max(abs(dx), abs(dy));

	// grab and compute the brush info
	int size = ui.brushSize;
	int w = activeBrush.width;
	int h = activeBrush.height;
	std::vector<int> mask = activeBrush.mask;

	int brushCenter_x = w / 2;
	int brushCenter_y = h / 2;

	for (int i = 0; i <= steps; i++)
	{
		// for reach row in the brush mask
		for (int r = 0; r < h; r++)
		{
			// for each column in the brush mask
			for (int c = 0; c < w; c++)
			{
				// if the current index is part of the pattern
				if (mask[r * w + c] == 1) 
				{
					for (int sy = 0; sy < size; sy++)
					{
						for (int sx = 0; sx < size; sx++)
						{
							// calculate the pixel x and y on the canvas
							int px = (lastX + dx * i / steps) + (c - brushCenter_x) * size + (sx - size / 2);
                        	int py = (lastY + dy * i / steps) + (r - brushCenter_y) * size + (sy - size / 2);;
							curCanvas.setPixel(px, py, ui.getColor());
						}
					}
				}
			}
		}
	}

	lastX = x;
	lastY = y;
} */