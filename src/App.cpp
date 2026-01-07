
#include "App.h"
#include "Window.h"
#include "Renderer.h"
#include "UI.h"
#include "Globals.h"
#include "CanvasManager.h"

static Window window; 
static Renderer renderer; 
static UI ui; 
static Globals global;
static CanvasManager canvasManager;

static int SCR_WIDTH = 1280;
static int SCR_HEIGHT = 720;


// create the window 
bool App::init() {

	if (!window.create(SCR_WIDTH, SCR_HEIGHT, "Capstone")) {
		return false; 
	}

	if (!renderer.init(window.handle(), global)) {
		return false; 
	}

	ui.init(window.handle(), renderer, global);

	//canvasManager.createCanvas(800, 600);

	return true; 

}


// run the render loop 
void App::run() 
{
	// on start up

	// RENDER LOOP 
	while (!window.shouldClose()) {
		window.pollEvents();

		// order of these four methods must not change
		renderer.beginFrame(canvasManager);
		ui.draw(canvasManager);
		renderer.endFrame();

		window.swapBuffers();
	}
}


// shut down the ui, renderer, and the window 
void App::shutdown() {
	ui.shutdown();
	renderer.shutdown();
	window.destroy();
}
