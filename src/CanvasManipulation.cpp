#include "CanvasManipulation.h"

#include "CanvasManipulation.h"
#include "CanvasManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glfw/glfw3.h>
#include "Globals.h"
#include <algorithm>

extern CanvasManager activeCanvasManager;
extern Globals global;

float lastAngle = 0.0f;


// panning funciton uses the delta of the x,y coordinates to change the offset of the canvas
void CanvasManipulation::panning(double deltaX, double deltaY)
{
	Canvas& canvas = activeCanvasManager.getActive();
	
	canvas.offset.x += (float)deltaX;
	canvas.offset.y -= (float)deltaY;
}

// inital calculation for the rotation upon press of the mouse
void CanvasManipulation::startRotate(double xpos, double ypos)
{
	Canvas& canvas = activeCanvasManager.getActive();

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f + canvas.offset.x,
		canvas.getHeight() * 0.5f + canvas.offset.y
	);

	lastAngle = atan2f(global.get_scr_height() - (float)ypos - canvasCenter.y, xpos - canvasCenter.x);
}

// rotating code that findes the center of the canvas to then calculate the 
// angle in radians to allow mouse and canvas to move along the screen together
void CanvasManipulation::rotating(double xpos, double ypos)
{
	Canvas& canvas = activeCanvasManager.getActive();

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f + canvas.offset.x,
		canvas.getHeight() * 0.5f + canvas.offset.y
	);

	float angle = atan2f(global.get_scr_height() - (float)ypos - canvasCenter.y, xpos - canvasCenter.x);

	canvas.rotation += angle - lastAngle;
	lastAngle = angle;
}




// zoom dragging works with curr ypos - lastYPos and a 0.0005 zoom speed
// the other forms of zooming use 1/-1 because that is what the scroll wheel produces, they also use a zoomspeed of 0.1
void CanvasManipulation::zooming(double yoffset, float zoomSpeed, double xpos, double ypos, GLFWwindow* window)
{
	Canvas& canvas = activeCanvasManager.getActive();

	// Rotate when holding R
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		canvas.rotation += (float)yoffset * 0.05f;
		return;
	}

	float oldZoom = canvas.zoom;

	canvas.zoom *= (1.0f + (float)yoffset * zoomSpeed);
	canvas.zoom = std::clamp(canvas.zoom, 0.1f, 10.0f);

	// Mouse position (screen space)

	viewMatrix(xpos, ypos, oldZoom, canvas);
}

// same as zooming but without the GLFWwindow for the check R for rotate
//void CanvasManipulation::zoomDragging(double yoffset, float zoomSpeed, double xpos, double ypos)
//{
//	Canvas& canvas = activeCanvasManager.getActive();
//
//	float oldZoom = canvas.zoom;
//
//	canvas.zoom *= (1.0f + (float)yoffset * zoomSpeed);
//	canvas.zoom = std::clamp(canvas.zoom, 0.1f, 10.0f);
//
//	// Mouse position (screen space)
//
//	viewMatrix(xpos, ypos, oldZoom, canvas);
//	std::cout << "testing" << std::endl;
//}


// this function is used to adjust the view of the canvas around the mouse instead of zooming just around the center 
void CanvasManipulation::viewMatrix(double x, double y, float oldZoom, Canvas& canvas)
{
	glm::vec2 mouseScreen(
		(float)x,
		(float)(global.get_scr_height() - y)
	);

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f,
		canvas.getHeight() * 0.5f
	);

	// --- Build OLD view matrix ---
	glm::mat4 oldView(1.0f);
	oldView = glm::translate(oldView, glm::vec3(canvas.offset, 0.0f));
	oldView = glm::translate(oldView, glm::vec3(canvasCenter, 0.0f));
	oldView = glm::rotate(oldView, canvas.rotation, glm::vec3(0, 0, 1));
	oldView = glm::scale(oldView, glm::vec3(oldZoom, oldZoom, 1.0f));
	oldView = glm::translate(oldView, glm::vec3(-canvasCenter, 0.0f));

	// Convert mouse to world/canvas space
	glm::vec4 world =
		glm::inverse(oldView) * glm::vec4(mouseScreen, 0.0f, 1.0f);

	// --- Build NEW view matrix ---
	glm::mat4 newView(1.0f);
	newView = glm::translate(newView, glm::vec3(canvas.offset, 0.0f));
	newView = glm::translate(newView, glm::vec3(canvasCenter, 0.0f));
	newView = glm::rotate(newView, canvas.rotation, glm::vec3(0, 0, 1));
	newView = glm::scale(newView, glm::vec3(canvas.zoom, canvas.zoom, 1.0f));
	newView = glm::translate(newView, glm::vec3(-canvasCenter, 0.0f));

	glm::vec4 newScreen = newView * world;
	canvas.offset += mouseScreen - glm::vec2(newScreen);
}

// centers the canvas to the screen
void CanvasManipulation::centerCamera()
{
	Canvas& canvas = activeCanvasManager.getActive();

	canvas.zoom = std::min((float)global.get_scr_width() / canvas.getWidth(), (float)global.get_scr_height() / canvas.getHeight()) * 0.95f;

	canvas.rotation = 0.0f;

	glm::vec2 screenCenter(
		global.get_scr_width() * 0.5f,
		global.get_scr_height() * 0.5f
	);

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f,
		canvas.getHeight() * 0.5f
	);

	canvas.offset = screenCenter - canvasCenter;
}