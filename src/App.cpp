
#include "App.h"
#include "Window.h"
#include "Renderer.h"
#include "UI.h"
#include "Globals.h"

static Window window; 
static Renderer renderer; 
static UI ui; 
static Globals global;

static int SCR_WIDTH = 1280;
static int SCR_HEIGHT = 720;


// create the window 
bool App::init() {

	if (!window.create(SCR_WIDTH, SCR_HEIGHT, "Capstone")) {
		return false; 
	}

	ui.init(window.handle(), renderer, global);

	if (!renderer.init(window.handle(), global)) {
		return false; 
	}

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
		unsigned int colorTexture = renderer.beginFrame();
		ui.draw();
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
