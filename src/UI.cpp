
#include "UI.h"
#include "Globals.h"
#include "CanvasManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>

// variables to store info for UI declared up here 
/// display size
float w, h;
/// panel sizes
int TopSize = 0;
int BotSize = 0;
int LeftSize = 0;
int RightSize = 0;

// state initial pop up 
static bool showPopup = false;

// show panels 
static bool showPanels = true;

// reference to the window, used for input 
static GLFWwindow* windowStorage;

// the init canvas values are displayed in the text boxes
static int canvasWidth = 1920;
static int canvasHeight = 1080;

// RBGA values for the color wheel 
static float color[4] = { .0f, .0f, .0f, 1.0f };

// storing time for user input 
static double lastFrame = 0.0;

// for storing the number of layers
int numLayers = 1;

// temp 
int UI::brushSize = 1;

// global instance reference
extern Globals global;

// state for draw erase button 
static UI::CursorMode cursorMode = UI::CursorMode::Draw; 

static Renderer renderer;


Color UI::getColor()
{
	Color c = {
		static_cast<unsigned char>(color[0] * 255.0f),
		static_cast<unsigned char>(color[1] * 255.0f),
		static_cast<unsigned char>(color[2] * 255.0f),
		static_cast<unsigned char>(color[3] * 255.0f)
	};

	return c;
}

// cursor mode getter 
UI::CursorMode UI::getCursorMode() const {
	return cursorMode;
}



// UI initialization 
void UI::init(GLFWwindow* window, Renderer& rendInst, Globals& g_inst) {
	renderer = rendInst;
	//global = g_inst;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// storing window for user input 
	windowStorage = window;
}


// NOTE: called in render loop 
void UI::draw(CanvasManager& canvasManager)
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
	if (TopSize == 0) { TopSize = static_cast<int>(0.05 * h); }
	if (BotSize == 0) { BotSize = static_cast<int>(0.05 * h); }
	if (LeftSize == 0) { LeftSize = static_cast<int>(0.1 * w); }
	if (RightSize == 0) { RightSize = static_cast<int>(0.1 * w); }

	// initial popup
	drawPopup(canvasManager);

	// -- user input to hide UI panels --
	if (glfwGetKey(windowStorage, GLFW_KEY_TAB) == GLFW_PRESS && glfwGetTime() - lastFrame >= 0.2) {
		showPanels = !showPanels;
		lastFrame = glfwGetTime();

	}

	// draw the four main menu panels
	if (showPanels) {
		drawLeftPanel(canvasManager);
		drawRightPanel(canvasManager);
		drawBottomPanel(canvasManager);
	}

	// top panel drawn regardless of input 
	drawTopPanel(canvasManager);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



// methods for drawing the individual menu panels
void UI::drawTopPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w, TopSize), ImGuiCond_Always);
	ImGui::Begin("Top Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	// add widgets
	// new canvas pop up
	if (ImGui::Button("New File")) {
		showPopup = true;
	}

	// end step
	ImGui::End();
}



void UI::drawLeftPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(LeftSize, h), ImGuiCond_Always);
	ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	/////// add widgets here ///////
	//ImGui::Text("TopSize = %d", TopSize); // <- here for debug
	// 
	// draw / erase buttons
	if (getCursorMode() == UI::CursorMode::Draw) {
		ImGui::Text("State: Draw");
	}

	else if (getCursorMode() == UI::CursorMode::Erase) {
		ImGui::Text("State: Erase");
	}

	if (ImGui::Button("Draw")) {
		cursorMode = UI::CursorMode::Draw;
	}

	if (ImGui::Button("Erase")) {
		cursorMode = UI::CursorMode::Erase;
	}

	// Save button
	if (ImGui::Button("Save")) {
		renderer.getFrameData(canvasManager);
	}

	// color wheel
	ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel |
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_AlphaBar;

	ImGui::ColorPicker4("", color, flags);

	// brush size slider 
	ImGui::SliderInt("Brush Size", &brushSize, 1, 2000);

	// end step
	LeftSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, h)); // keeps its Y-value the same
	ImGui::End();
}


void UI::drawRightPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(w - RightSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(RightSize, h - TopSize), ImGuiCond_Always);
	ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	// add widgets
	if (canvasManager.hasActive())
	{
		ImGui::Text("file is open");
		ImGui::Text("file size is: ");
		ImGui::Text("%dx%d", canvasManager.getActive().getWidth(), canvasManager.getActive().getHeight());
		
		// Create the layer buttons
		if(ImGui::Button("New Layer")){
			// increase the number of layers by 1
			numLayers++;
		}
		// 
		if(ImGui::Button("Remove Layer")){
			if(numLayers > 1){
				// decrease the number of layers by 1
				numLayers--;
				// call a function that adds a layer
				
			}
		}

		for(int i = 0; i < numLayers; i++){
			std::string buttonName = "Canvas Layer " + std::to_string(i);
			if(ImGui::Button(buttonName.c_str())){
				// call function that adds a layer
				std::cout << "new layer selected" << std::endl;
			}
		}
	}


	// end step
	RightSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, h)); // keeps its Y-value the same
	ImGui::End();
}


void UI::drawBottomPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, h - BotSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w - LeftSize - RightSize, BotSize), ImGuiCond_Always);
	ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	// add widgets

	// end step
	ImGui::End();
}

// canvas size popup 
void UI::drawPopup(CanvasManager& canvasManager)
{
	static int temp_w = 1920;
	static int temp_h = 1080;

	if (showPopup) {
		ImGui::OpenPopup("New Canvas");
	}

	// specifying canvas size 
	if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::InputInt("Width", &temp_w);
		ImGui::InputInt("Height", &temp_h);

		// if user creates a canvas, remove the popup 
		if (ImGui::Button("Create")) {

			// create the new canvas
			canvasManager.createCanvas(temp_w, temp_h);


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