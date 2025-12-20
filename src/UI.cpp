
#include "UI.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// variables to store info for UI declared up here 

// state initial pop up 
static bool showPopup = true; 
// the init canvas values are displayed in the text boxes
static int canvasWidth = 1920; 
static int canvasHeight = 1080; 


// state for draw erase button 
enum Mode {
	DRAW, 
	ERASE
};

static Mode drawState = DRAW;  


// UI initialization 

void UI::init(GLFWwindow* window) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark(); 

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410 core");
}


// buttons are drawn here 

void UI::draw() {

	// start ImGui frame before adding widgets 
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// draw erase button 
	drawDrawEraseButton(); 

	// initial popup
	drawPopup();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// methods for drawing the individual widgets 
void UI::drawDrawEraseButton() {

	ImGui::Begin("Side Panel");

	if (drawState == DRAW) {
		ImGui::Text("Current State: Draw");
	}
	else {
		ImGui::Text("Current State: Erase");
	}

	if (ImGui::Button("Draw")) {
		drawState = DRAW;
	}

	if (ImGui::Button("Erase")) {
		drawState = ERASE;
	}

	ImGui::End();
}


void UI::drawPopup() {
	if (showPopup) {
		ImGui::OpenPopup("New Canvas"); // match name w/ BeginPopupModal
		showPopup = false;
	}

	// specifying canvas size 
	if (ImGui::BeginPopupModal("New Canvas", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		// canvas size inputs 
		ImGui::InputInt("Width", &canvasWidth);
		ImGui::InputInt("Height", &canvasHeight);

		// if user creates a canvas, remove the popup 
		if (ImGui::Button("Create")) {
			showPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			showPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


// ending and cleanup 
void UI::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown(); 
	ImGui::DestroyContext();
}