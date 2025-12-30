
#include "UI.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

// variables to store info for UI declared up here 
/// display size
float w, h;
/// panel sizes
int TopSize = 0;
int BotSize = 0;
int LeftSize = 0;
int RightSize = 0;

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
Renderer renderer;

// RBGA values for the color wheel 
static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// UI initialization 
void UI::init(GLFWwindow* window, Renderer rendInst) {
	renderer = rendInst;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark(); 

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410 core");
}

void UI::draw(unsigned int colorTexture) 
{
	// start ImGui frame before adding widgets 
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// grab the window display size
	ImGuiIO& io = ImGui::GetIO();
	w = io.DisplaySize.x;
	h = io.DisplaySize.y;

	// compute the panel sizes
	if (TopSize == 0) {TopSize = static_cast<int>(0.05 * h);}
	if (BotSize == 0) {BotSize = static_cast<int>(0.05 * h);}
	if (LeftSize == 0) {LeftSize = static_cast<int>(0.1 * w);}
	if (RightSize == 0) {RightSize = static_cast<int>(0.1 * w);}

	// initial popup
	drawPopup();

	// draw the four main menu panels
	drawTopPanel();
	drawLeftPanel();
	drawRightPanel();
	drawBottomPanel();

	// draw the center canvas
	drawCenterCanvas(colorTexture);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// methods for drawing the individual menu panels
void UI::drawTopPanel() {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w, TopSize), ImGuiCond_Always);
	ImGui::Begin("Top Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	// add widgets

	// end step
	ImGui::End();
}

void UI::drawLeftPanel() {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(LeftSize, h), ImGuiCond_Always);
	ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	/////// add widgets here ///////
	//ImGui::Text("TopSize = %d", TopSize); // <- here for debug
	// draw / erase buttons
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

	// Save button
	if (ImGui::Button("Save")){
		renderer.getFrameData();
	}

	// color wheel
	ImGui::ColorEdit4("", color, ImGuiColorEditFlags_PickerHueWheel);

	// end step
	LeftSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, h)); // keeps its Y-value the same
	ImGui::End();
}

void UI::drawRightPanel() {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(w - RightSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(RightSize, h - TopSize), ImGuiCond_Always);
	ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	
	// add widgets

	// end step
	RightSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, h)); // keeps its Y-value the same
	ImGui::End();
}

void UI::drawBottomPanel() {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, h - BotSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w - LeftSize - RightSize, BotSize), ImGuiCond_Always);
	ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	// add widgets

	// end step
	ImGui::End();
}

void UI::drawCenterCanvas(unsigned int colorTexture) {
	// panel settings
	int fbWidth = w - LeftSize - RightSize;
	int fbHeight = h - TopSize - BotSize;

	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(fbWidth, fbHeight), ImGuiCond_Always);
	ImGui::Begin("OpenGL Framebuffer", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

	// add widgets
	// Display framebuffer
        ImGui::Image(
            (void*)(intptr_t)colorTexture,
            ImVec2((float)fbWidth, (float)fbHeight),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

	// end step
	ImGui::End();
}

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

void UI::drawColorWheel() {
	ImGui::Begin("Color");

	ImGui::ColorEdit4("Color Picker", color, ImGuiColorEditFlags_PickerHueWheel);
	//ImGui::Text("RGBA: %.2f,%.2f,%.2f,%.2f", color[0], color[1], color[2], color[3]);

	ImGui::End();
}


// ending and cleanup 
void UI::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown(); 
	ImGui::DestroyContext();
}