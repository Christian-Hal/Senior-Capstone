
#include "App.h"
#include "Window.h"
#include "Renderer.h"
#include "UI.h"

static Window window; 
static Renderer renderer; 
static UI ui; 

bool App::init() {

	if (!window.create(800, 600, "Capstone")) {
		return false; 
	}

	if (!renderer.init(window.handle())) {
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
		renderer.beginFrame();
		ui.draw();
		renderer.endFrame();

		window.swapBuffers();
	}
}

void App::shutdown() {
	ui.shutdown();
	renderer.shutdown();
	window.destroy();
}