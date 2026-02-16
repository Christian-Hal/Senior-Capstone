
#include "UI.h"

#include <iostream>
#include <string>

#include "Globals.h"
#include "CanvasManager.h"
#include "FrameRenderer.h"
#include "BrushManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_stdlib.h>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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

// brush size 
int UI::brushSize = 1;

// mouse flags 
static ImGuiConfigFlags cursorFlags; 

// for imported images  
int my_image_width = 0; 
int my_image_height = 0; 
GLuint my_image_texture = 0; 

// global instance reference
extern Globals global;

// state for draw erase button 
static UI::CursorMode cursorMode = UI::CursorMode::Draw; 

static Renderer renderer;

// brush manager
extern BrushManager brushManager;

// keeping track of popup status 
static bool popupActive; 



// ----- ImGui code to load and access images in directory -----


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}



// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
	FILE* f = fopen(file_name, "rb");
	if (f == NULL)
		return false;
	fseek(f, 0, SEEK_END);
	size_t file_size = (size_t)ftell(f);
	if (file_size == -1)
		return false;
	fseek(f, 0, SEEK_SET);
	void* file_data = IM_ALLOC(file_size);
	fread(file_data, 1, file_size, f);
	fclose(f);
	bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
	IM_FREE(file_data);
	return ret;
}



Color UI::getColor()
{
	if (cursorMode == UI::CursorMode::Draw) 
	{	
		return Color{
			static_cast<unsigned char>(color[0] * 255.0f),
			static_cast<unsigned char>(color[1] * 255.0f),
			static_cast<unsigned char>(color[2] * 255.0f),
			static_cast<unsigned char>(color[3] * 255.0f)
		};
	}
	if (cursorMode == UI::CursorMode::Erase) 
	{	
		return Color{
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f)
		};
	}
}

/*
	Setter for color

	@param currentPixelColor: The color of the pixel that the cursor is currently 
						      hovering over. 
*/
void UI::setColor(Color currentPixelColor) {
	color[0] = static_cast<float>(currentPixelColor.r);
	color[1] = static_cast<float>(currentPixelColor.g);
	color[2] = static_cast<float>(currentPixelColor.b);
	color[3] = static_cast<float>(currentPixelColor.a);

	cout << "r set to " << color[0] << endl; 
	cout << "g set to " << color[1] << endl; 
	cout << "b set to " << color[2] << endl; 
	cout << "a set to " << color[3] << endl; 

}



// cursor mode getter 
UI::CursorMode UI::getCursorMode() const {
	return cursorMode;
}



void UI::setCursorMode(UI::CursorMode temp) {
	cursorMode = temp;
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
void UI::draw(CanvasManager& canvasManager, FrameRenderer frameRenderer)
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
	// only allow this if the canvas creation popup is not active 
	if (!ImGui::IsPopupOpen("New Canvas")) {
		if (glfwGetKey(windowStorage, GLFW_KEY_TAB) == GLFW_PRESS && glfwGetTime() - lastFrame >= 0.2) {
			showPanels = !showPanels;
			lastFrame = glfwGetTime();

		}
	}
	

	// draw the four main menu panels
	if (showPanels) {
		drawLeftPanel(canvasManager);
		drawRightPanel(canvasManager);
		drawBottomPanel(canvasManager, frameRenderer);
	}

	// top panel drawn regardless of input 
	drawTopPanel(canvasManager);

	// canvas tab panel shown only if more than 1 canvas is open
	if (canvasManager.getNumCanvases() > 1) { drawCanvasTabs(canvasManager); }


	// ----- Cursor Customization -----

	// keep default cursor when no canvas 
	if (!(ImGui::GetIO().WantCaptureMouse) && canvasManager.getNumCanvases() > 0) {
		// erase standard mouse
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		// grab our brush and its tip 
		BrushTool activeBrush = brushManager.getActiveBrush();
		const std::vector<float>& tipAlpha = activeBrush.tipAlpha;

		// check if tipAlpha actually has data
		if (tipAlpha.empty()) return;

		int tw = activeBrush.tipWidth;
		int th = activeBrush.tipHeight;
		float scale = (float)UI::brushSize;

		// grab mouse position and initialize draw list 
		ImVec2 mousePos = ImGui::GetMousePos();

		// *** TEMP FIX ***
		// drawing the custom cursor only if the brushsize is large enough
		if (scale >= 3) {
			ImDrawList* drawList = ImGui::GetForegroundDrawList();

			// center brush tip
			// NOTE: once we are able to draw from a single click, the float values in these 
			// calculations might need to be slightly changed 
			ImVec2 offset = ImVec2(mousePos.x - (tw * scale * 0.3f), mousePos.y - (th * scale * 0.3f));

			for (int y = 0; y < th; y++) {
				for (int x = 0; x < tw; x++) {
					int i = y * tw + x;

					// edge pixels, for the cursor outline, are only those with alpha values above a threshold
					if (tipAlpha[i] > 0.01f) {
						ImVec2 p_min = ImVec2(offset.x + (x * scale * 0.63), offset.y + (y * scale * 0.63));
						ImVec2 p_max = ImVec2(p_min.x + scale, p_min.y + scale);

						// drawing those pixels, color value is fix
						drawList->AddRect(p_min, p_max, IM_COL32(128, 128, 128, 255));
					}
				}
			}
		}

		// if brush size is smaller, then draw default circle cursor
		else {
			ImGui::GetForegroundDrawList()->AddCircle(ImGui::GetMousePos(), 10, IM_COL32(255, 0, 0, 255));
		}

		
	}

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
	// draw / erase buttons
	if (getCursorMode() == UI::CursorMode::Draw) {
		ImGui::Text("State: Draw");
	}

	else if (getCursorMode() == UI::CursorMode::Erase) {
		ImGui::Text("State: Erase");
	}

	else if (getCursorMode() == UI::CursorMode::ZoomIn) {
		ImGui::Text("State: Zoom In");
	}

	else if (getCursorMode() == UI::CursorMode::ZoomOut) {
		ImGui::Text("State: Zoom Out");
	}

	else if (getCursorMode() == UI::CursorMode::Rotate) {
		ImGui::Text("State: Rotate");
	}

	else if (getCursorMode() == UI::CursorMode::Pan) {
		ImGui::Text("State: Pan");
	}

	else if (getCursorMode() == UI::CursorMode::ColorPick) {
		ImGui::Text("State: Color Pick"); 
	}



	if (ImGui::Button("Draw")) {
		cursorMode = UI::CursorMode::Draw;
	}

	if (ImGui::Button("Erase")) {
		cursorMode = UI::CursorMode::Erase;
	}

	if (ImGui::Button("Pan")) {
		cursorMode = UI::CursorMode::Pan;
	}

	if (ImGui::Button("Rotate")) {
		cursorMode = UI::CursorMode::Rotate;
	}

	if (ImGui::Button("Zoom In")) {
		cursorMode = UI::CursorMode::ZoomIn;
	}

	if (ImGui::Button("Zoom Out")) {
		cursorMode = UI::CursorMode::ZoomOut;
	}

	if (ImGui::Button("Color Picker")) {
		cursorMode = UI::CursorMode::ColorPick; 
	}
	

	// Save button
	if (ImGui::Button("Save")) {
		canvasManager.getFrameData(canvasManager);
	}

	// color wheel
	ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel |
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_AlphaBar;

	ImGui::ColorPicker4("", color, flags);

	// brush size slider 
	// value is temporarily significantly lowered due to current brush size implementation 
	ImGui::SliderInt("Brush Size", &brushSize, 1, 20);

	// --- Displaying loaded brush options ---
	// grabs the list of loaded brushes
	const std::vector<BrushTool>& brushes = brushManager.getLoadedBrushes();

	// adds a button for each brush that sets it to the active one
	for(int i = 0; i < brushes.size(); i++)
	{
		std::string buttonName = brushes[i].brushName;

		if(ImGui::Button(buttonName.c_str())){
			brushManager.setActiveBrush(i);
		}
	}

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
		// save the active canvas for later use
		ImGui::Text("file is open");
		ImGui::Text("file size is: ");
		ImGui::Text("%dx%d", canvasManager.getActive().getWidth(), canvasManager.getActive().getHeight());
		
		// Create the layer buttons
		if(ImGui::Button("New Layer")){
			// increase the number of layers by 1
			canvasManager.getActive().createLayer();
		}
		// remove a layer button 
		
		if(ImGui::Button("Remove Layer")){
			if(canvasManager.getActive().getNumLayers() > 2){
				// decrease the number of layers by 1
				canvasManager.getActive().removeLayer();				
			}
		}

		for(int i = 1; i < canvasManager.getActive().getNumLayers(); i++){
			std::string buttonName = "Canvas Layer " + std::to_string(i);
			if(ImGui::Button(buttonName.c_str())){
				canvasManager.getActive().selectLayer(i);
			}
		}
		ImGui::Text("Current canvas: %d", canvasManager.getActive().getCurLayer());
	}


	// end step
	RightSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, h)); // keeps its Y-value the same
	ImGui::End();
}



void UI::drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, h - BotSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w - LeftSize - RightSize, BotSize), ImGuiCond_Always);
	ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	// add widgets
	ImGui::Text("UNFINISHED");
	if(ImGui::Button("Next Frame")){
		FrameRenderer::selectFrame(canvasManager.getActive(), 1);
	}
	if(ImGui::Button("Previous Frame")){
		FrameRenderer::selectFrame(canvasManager.getActive(), -1);
	}

	if(ImGui::Button("Create Frame")){
		FrameRenderer::createFrame(canvasManager.getActive());
	}
	if(ImGui::Button("Remove Frame")){
		FrameRenderer::removeFrame(canvasManager.getActive());
	}
	if(ImGui::Button("Play")){
		FrameRenderer::play(canvasManager.getActive());
	}
	// end step
	if (ImGui::GetWindowHeight() > h - 2 * TopSize)
		BotSize = h - 2 * TopSize;
	else
		BotSize = ImGui::GetWindowHeight();
	ImGui::End();
}



void UI::drawCanvasTabs(CanvasManager& canvasManager)
{
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w - LeftSize - RightSize, TopSize), ImGuiCond_Always);
	ImGui::Begin("Canvas Tabs Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	// add widgets

	// --- Displaying the loaded canvases ---
	// grabs the list of loaded canvases
	const std::vector<Canvas>& canvases = canvasManager.getOpenCanvases();

	// adds a button for each canvas that sets it to the active one
	for(int i = 0; i < canvasManager.getNumCanvases(); i++)
	{
		std::string buttonName = canvases[i].getName();

		if(ImGui::Button(buttonName.c_str())){
			canvasManager.setActiveCanvas(i);
		}

		ImGui::SameLine();
	}

	// end step
	ImGui::End();
}



// canvas size popup 
void UI::drawPopup(CanvasManager& canvasManager)
{
	static int temp_w = 1920;
	static int temp_h = 1080;
	static std::string temp_n = "Untitled";

	if (showPopup) {
		ImGui::OpenPopup("New Canvas");
	}

	// specifying canvas size 
	if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		ImGui::InputInt("Width:", &temp_w);
		ImGui::InputInt("Height:", &temp_h);
		ImGui::InputText("File Name:", &temp_n);

		// if user creates a canvas, remove the popup 
		if (ImGui::Button("Create")) {

			// create the new canvas
			canvasManager.createCanvas(temp_w, temp_h, temp_n);

			showPopup = false;
			temp_n = "Untitled";

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			showPopup = false;
			temp_n = "Untitled";
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