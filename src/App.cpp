
#include "App.h"
#include "Window.h"
#include "Renderer.h"
#include "UI.h"

static Window window; 
static Renderer renderer; 
static UI ui; 

static int SCR_WIDTH = 1280;
static int SCR_HEIGHT = 720;

bool App::init() {

	if (!window.create(SCR_WIDTH, SCR_HEIGHT, "Capstone")) {
		return false; 
	}

	if (!renderer.init(window.handle(), SCR_WIDTH, SCR_HEIGHT)) {
		return false; 
	}

	ui.init(window.handle());
	return true; 

}


void App::run() {

	// RENDER LOOP 
	while (!window.shouldClose()) {
		window.pollEvents();

		// order of these four methods must not change
		unsigned int colorTexture = renderer.beginFrame();
		ui.draw(colorTexture);
		renderer.endFrame();

		window.swapBuffers();
	}
}

void App::shutdown() {
	ui.shutdown();
	renderer.shutdown();
	window.destroy();
}