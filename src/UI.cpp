
#include "UI.h"
#include "Globals.h"
#include "CanvasManager.h"

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
static bool showPopup = false;

// show panels 
static bool showPanels = true;

// reference to the window, used for input 
static GLFWwindow* windowStorage;

// the init canvas values are displayed in the text boxes
static int canvasWidth = 0;
static int canvasHeight = 0;

// RBGA values for the color wheel 
static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// storing time for user input 
static double lastFrame = 0.0;


static Globals global;

// state for draw erase button 
enum Mode {
	DRAW,
	ERASE
};


static Mode drawState = DRAW;
static Renderer renderer;


//struct ColorI
//{
//	uint8_t r, g, b, a;
//};

Color UI::getColor()
{
	Color c = {
		static_cast<int>(color[0] * 255.0f),
		static_cast<int>(color[1] * 255.0f),
		static_cast<int>(color[2] * 255.0f),
		static_cast<int>(color[3] * 255.0f)
	};

	return c;
}

//int UI::getColor()
//{
//	int returnCol[4] = {
//		static_cast<int>(color[0] * 255.0f),
//		static_cast<int>(color[1] * 255.0f),
//		static_cast<int>(color[2] * 255.0f),
//		static_cast<int>(color[3] * 255.0f)
//	};
//
//	return returnCol[4];
//}



// UI initialization 
void UI::init(GLFWwindow* window, Renderer& rendInst, Globals& g_inst) {
	renderer = rendInst;
	global = g_inst;

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
	// draw / erase buttons
	if (drawState == DRAW) {
		ImGui::Text("State: Draw");
	}
	else {
		ImGui::Text("State: Erase");
	}

	if (ImGui::Button("Draw")) {
		drawState = DRAW;
	}

	if (ImGui::Button("Erase")) {
		drawState = ERASE;
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
	static int temp_w = 0;
	static int temp_h = 0;

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