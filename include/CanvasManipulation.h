
#pragma once
#include <glfw/glfw3.h>

class Globals;
class Canvas;

class CanvasManipulation {
public:
	void zooming(double yoffset, float zoomSpeed, double xpos, double ypos, GLFWwindow* window);
	//void zoomDragging(double yoffset, float zoomSpeed, double xpos, double ypos);
	void panning(double deltaX, double deltaY);
	void startRotate(double xpos, double ypos);
	void rotating(double xpos, double ypos);
	void centerCamera();

private:
	void viewMatrix(double x, double y, float oldZoom, Canvas& canvas);
};
