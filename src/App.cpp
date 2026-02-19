
#include "App.h"

#include "Window.h"
#include "Renderer.h"
#include "UI.h"
#include "Globals.h"
#include "CanvasManager.h"
#include "BrushManager.h"
#include "FrameRenderer.h"

#include "InputManager.h"


// define our static objects and vars 
static Window window; 
static Renderer renderer; 
static UI ui; 
Globals global;
BrushManager brushManager;
static CanvasManager canvasManager;
FrameRenderer frameRenderer;

static InputManager inputManager;

static int SCR_WIDTH = 1280;
static int SCR_HEIGHT = 720;


/*
Creates the window according to screen height and width values.

Initializes the UI.

@return true if window was successfully created.
*/
bool App::init() {

	
	if (!window.create(SCR_WIDTH, SCR_HEIGHT, "Capstone", global)) {
		return false; 
	}

	if (!renderer.init(window.handle(), global)) {
		return false; 
	}

	brushManager.init();
	ui.init(window.handle(), renderer, global);

	//inputManager.init(window.handle());

	global.set_scr_width(SCR_WIDTH);
	global.set_scr_height(SCR_HEIGHT);

	return true; 

}



/*
Runs the render loop.

Begin frame, draw UI, end frame.
*/
void App::run() 
{
	// on start up

	// RENDER LOOP 
	while (!window.shouldClose()) {
		window.pollEvents();

		// order of these four methods must not change
		renderer.beginFrame(canvasManager);
		ui.draw(canvasManager, frameRenderer);
		renderer.endFrame();
		inputManager.update();

		window.swapBuffers();
	}
}



/*
Shuts down the UI, renderer, and the window.
*/
void App::shutdown() {
	frameRenderer.shutdown();
	ui.shutdown();
	renderer.shutdown();
	window.destroy();
}
