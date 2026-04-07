#include "UI.h"

#include <iostream>
#include <fstream>
#include <string>
#include <utility>

#include "ImGuiFileDialog.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_stdlib.h>


#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


std::string overwritePath;

// variables to store info for UI declared up here 
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

// mouse flags 
static ImGuiConfigFlags cursorFlags;

// for imported images  
int my_image_width = 0;
int my_image_height = 0;
GLuint my_image_texture = 0;

// state for draw erase button 
static CursorMode cursorMode = CursorMode::Draw;


void UI::bindCursorCallbacks(SetCursorModeCallback setCb, GetCursorModeCallback getCb)
{
	// Store controller-provided hooks so UI can request state changes
	// without depending on AppController/AppState directly.
	setCursorModeCb = std::move(setCb);
	getCursorModeCb = std::move(getCb);
}


void UI::bindCanvasCallbacks(ResetCanvasPositionCallback resetPositionCb)
{
	resetCanvasPositionCb = std::move(resetPositionCb);
}


void UI::bindBrushCallbacks(GetBrushListCallback getListCb, SetActiveBrushCallback setActiveCb, GetActiveBrushCallback getActiveCb, LoadBrushCallback loadBrushCb, GenerateBrushDabCallback genDabCb)
{
	getBrushListCb = std::move(getListCb);
	setActiveBrushCb = std::move(setActiveCb);
	getActiveBrushCb = std::move(getActiveCb);
	loadBrushFromFileCb = std::move(loadBrushCb);
	generateDabCb = std::move(genDabCb);
}


void UI::bindHotkeyCallbacks(GetHotkeyLabelCallback getLabelCb, StartRebindCallback startCb, BoolCallback isWaitingCb, BoolCallback didFailCb)
{
	getHotkeyLabelCb = std::move(getLabelCb);
	startRebindCb = std::move(startCb);
	isWaitingForRebindCb = std::move(isWaitingCb);
	didRebindFailCb = std::move(didFailCb);
}

void UI::bindRecentActivityCallbacks(saveToRecentActivityCallback saveCb, getRecentActivityCallback getCb)
{
	saveToRecentActivityCb = std::move(saveCb);
	getRecentActivityCb = std::move(getCb);
}


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


// note: not all options here return a value. 
Color UI::getColor()
{
	const CursorMode mode = getCursorMode();

	if (mode == CursorMode::Draw)
	{
		return Color{
			static_cast<unsigned char>(color[0] * 255.0f),
			static_cast<unsigned char>(color[1] * 255.0f),
			static_cast<unsigned char>(color[2] * 255.0f),
			static_cast<unsigned char>(color[3] * 255.0f)
		};
	}
	if (mode == CursorMode::Erase)
	{
		return Color{
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f),
			static_cast<unsigned char>(0.f)
		};
	}

	return Color{ 0, 0, 0, 0 };
}


/*
	Setter for color

	@param currentPixelColor: The color of the pixel that the cursor is currently
							  hovering over.
*/
void UI::setColor(Color currentPixelColor) {
	color[0] = currentPixelColor.r / 255.0f;
	color[1] = currentPixelColor.g / 255.0f;
	color[2] = currentPixelColor.b / 255.0f;
	color[3] = currentPixelColor.a / 255.0f;


}


// cursor mode getter 
CursorMode UI::getCursorMode() const {
	// Prefer controller-owned state once callbacks are bound.
	if (getCursorModeCb) {
		return getCursorModeCb();
	}

	return cursorMode;
}

void UI::setCursorMode(CursorMode temp) {
	// Keep local fallback in sync for transitional code paths.
	cursorMode = temp;

	if (setCursorModeCb) {
		setCursorModeCb(temp);
	}
}


// UI initialization 
void UI::init(GLFWwindow* window, Renderer& rendInst, Globals& g_inst) {

	// set the initial ui state
	curState = UIState::start_menu;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiStyle& style = ImGui::GetStyle();

	// Clip Studio-inspired grayscale UI styling.
	//style.WindowRounding = 5.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 6.0f;
	style.FramePadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);

	colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11f, 0.11f, 0.11f, 0.90f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);

	colors[ImGuiCol_SliderGrab] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);

	colors[ImGuiCol_Button] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);

	colors[ImGuiCol_Separator] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);

	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.35f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.48f, 0.48f, 0.48f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.60f, 0.60f, 0.60f, 0.95f);

	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.92f, 0.92f, 0.92f, 0.95f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.95f, 0.95f, 0.95f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.40f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// storing window for user input 
	windowStorage = window;

}


// NOTE: called in render loop 
void UI::drawUI(CanvasManager& canvasManager, FrameRenderer frameRenderer)
{
	// start ImGui frame before adding widgets 
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// grab the window display size
	ImGuiIO& io = ImGui::GetIO();
	displayWidth = io.DisplaySize.x;
	displayHeight = io.DisplaySize.y;

	// compute the panel sizes if needed
	// they should only ever be 0 at init, so this is really only called on the first time drawn
	if (TopSize == 0) { TopSize = static_cast<int>(0.05 * displayHeight); }
	if (BotSize == 0) { BotSize = static_cast<int>(0.05 * displayHeight); }
	if (LeftSize == 0) { LeftSize = static_cast<int>(0.1 * displayWidth); }
	if (RightSize == 0) { RightSize = static_cast<int>(0.1 * displayWidth); }

	// ----- Call the various popup draws so they get drawn when needed -----
	drawNewCanvasPopup(canvasManager); // draw the new canvas pop up

	// ----- Cursor Customization -----
	drawCustomCursor(canvasManager);

	// draw ui based on the UI's current state
	if (curState == UIState::start_menu)		{drawStartScreen(canvasManager);}
	else if (curState == UIState::main_screen) 	{drawMainScreen(canvasManager, frameRenderer);}

	// top panel is drawn regardless of the state 
	drawTopPanel(canvasManager);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::drawStartScreen(CanvasManager& canvasManager)
{
	// panel sizes for start screen
	int start_left = static_cast<int>(0.25 * displayWidth);
	int start_right = static_cast<int>(0.75 * displayWidth);

	// draw the left part of the start screen (save and load buttons)
	ImGui::SetNextWindowPos(ImVec2(0, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(start_left, displayHeight), ImGuiCond_Always);
	ImGui::Begin("Left Start Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImVec2 buttonSize = ImVec2(160, 42); 	// makes the buttons bigger
	float centerX = (start_left - 160) * 0.5f;

	ImGui::Dummy(ImVec2(0, 40)); 			// creates some vertical space
	ImGui::SetCursorPosX(centerX); 			// center the button

	if (ImGui::Button("Create File", buttonSize)) 
	{
		showPopup = true;
	}

	ImGui::Dummy(ImVec2(0, 10)); 			// creates some vertical space
	ImGui::SetCursorPosX(centerX); 			// center the button

	if (ImGui::Button("Open File", buttonSize))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			"LoadFileDlg",
			"Choose File",
			".png,.jpg,.ora"
		);
	}

	if (ImGuiFileDialog::Instance()->Display("LoadFileDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath =
				ImGuiFileDialog::Instance()->GetFilePathName();

			std::cout << "filepath" << std::endl;

			std::string extension =
				ImGuiFileDialog::Instance()->GetCurrentFilter();

			std::cout << "extension" << std::endl;

			if (extension == ".ora")
			{
				canvasManager.loadORA(filePath);
				// centering the loaded image 
				resetCanvasPositionCb();
			}
			else
			{
				canvasManager.loadFromFile(filePath);
				// centering the loaded image 
				resetCanvasPositionCb();
			}

			// save to recent activity list
			saveToRecentActivityCb(filePath);

			// if the current UI state is the start menu then change it to the main screen
			if (curState == UIState::start_menu) {curState = UIState::main_screen;}
			
		}

		ImGuiFileDialog::Instance()->Close();
	}

	// end step
	ImGui::End();

	// draw the right part of the start screen (recent activity)
	ImGui::SetNextWindowPos(ImVec2(start_left, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(start_right, displayHeight), ImGuiCond_Always);
	ImGui::Begin("Right Start Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImGui::Text("Recent Actvity: ");

	// adds a little visual split between sections
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// display recent activity list
	if (getRecentActivityCb) {
		const std::vector<std::string>& recentActivity = getRecentActivityCb();
		for (const std::string& filePath : recentActivity) {
			// grab the filename from the file path for display
			size_t lastSlash = filePath.find_last_of("/\\");
			std::string fileName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

			// if the button is clicked then load in the file
			if (ImGui::Button(fileName.c_str(), ImVec2(150, 0))) {
				std::string extension = filePath.substr(filePath.find_last_of('.'));

				if (extension == ".ora") {
					canvasManager.loadORA(filePath);
					// centering the loaded image 
					resetCanvasPositionCb();
				}
				else {
					canvasManager.loadFromFile(filePath);
					// centering the loaded image 
					resetCanvasPositionCb();
				}

				// if the current UI state is the start menu then change it to the main screen
				if (curState == UIState::start_menu) {curState = UIState::main_screen;}
			}
		}
	}

	// end step
	ImGui::End();
}

void UI::drawMainScreen(CanvasManager& canvasManager, FrameRenderer frameRenderer) 
{
	// -- user input to hide UI panels --
	// only allow this if the canvas creation popup is not active 
	if (!ImGui::IsPopupOpen("New Canvas")) {
		if (glfwGetKey(windowStorage, GLFW_KEY_TAB) == GLFW_PRESS && glfwGetTime() - lastFrame >= 0.2) {
			showPanels = !showPanels;
			lastFrame = glfwGetTime();

		}
	}

	// draw the three main screen panels
	if (showPanels) {
		drawLeftPanel(canvasManager);
		drawRightPanel(canvasManager);
		drawBottomPanel(canvasManager, frameRenderer);
	}

	// canvas tab panel shown only if more than 1 canvas is open
	if (canvasManager.getNumCanvases() > 1) { drawCanvasTabs(canvasManager); }
}

void UI::drawCustomCursor(CanvasManager& canvasManager) {
	// only use custom cursor if a canvas is open, we are hovering over it, and we are drawing/erasing
	if (ImGui::GetIO().WantCaptureMouse || canvasManager.getNumCanvases() <= 0) return;

	if (UI::getCursorMode() == CursorMode::Draw || UI::getCursorMode() == CursorMode::Erase) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		// grabbing the brush size from BrushManager
		std::vector<float> dab = generateDabCb(brushSize);
		if (dab.size() < 2) return;

		// grabbing the dimensions at the start of dab vector 
		int W = static_cast<int>(dab[0]);
		int H = static_cast<int>(dab[1]);

		// grabbing the zoom to maintain cursor size consistency 
		float zoom = canvasManager.getActive().zoom;

		ImVec2 mousePos = ImGui::GetMousePos();
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		// accounting for zoom 
		float displayW = W * zoom;
		float displayH = H * zoom;

		// calculating and centering offset by subtracting half the width/height of the GENERATED dab
		ImVec2 origin = ImVec2(mousePos.x - (displayW * 0.5f), mousePos.y - (displayH * 0.5f));

		if (brushSize >= 3) {
			for (int y = 0; y < H; y++) {
				int flippedY = H - 1 - y;
				for (int x = 0; x < W; x++) {
					// bro idk what this evil magic line of code is 
					float alpha = dab[2 + (flippedY * W + x)];

					// only drawing pixels that are part of the brush tip shape
					if (alpha > 0.1f) {
						ImVec2 p_min = ImVec2(origin.x + (x * zoom), origin.y + (y * zoom));
						ImVec2 p_max = ImVec2(p_min.x + zoom, p_min.y + zoom);

						// setting the cursor color 
						drawList->AddRectFilled(p_min, p_max, IM_COL32(128, 128, 128, 150));
					}
				}
			}
		}
		else {
			ImVec2 center = ImVec2(origin.x + (displayW * 0.5f), origin.y + (displayH * 0.5f));

			float brushRadius = (displayW * 0.5f); 

			// drawing the circle 
			drawList->AddCircle(center, brushRadius, IM_COL32(255, 255, 255, 100), 32); 

			// drawing the crosshair 
			float crossSize = 12.0f; 
			float gap = 4.0f; 

			ImU32 colorWhite = IM_COL32(255, 255, 255, 255); 
			ImU32 colorBlack = IM_COL32(0, 0, 0, 255); 

			auto drawOutlinedLine = [&](ImVec2 p1, ImVec2 p2) {
				drawList->AddLine(p1, p2, colorBlack, 3.0f);
				drawList->AddLine(p1, p2, colorWhite, 1.0f);
				};

			drawOutlinedLine(ImVec2(mousePos.x, mousePos.y - gap), ImVec2(mousePos.x, mousePos.y - crossSize));
			drawOutlinedLine(ImVec2(mousePos.x, mousePos.y + gap), ImVec2(mousePos.x, mousePos.y + crossSize));
			drawOutlinedLine(ImVec2(mousePos.x - gap, mousePos.y), ImVec2(mousePos.x - crossSize, mousePos.y));
			drawOutlinedLine(ImVec2(mousePos.x + gap, mousePos.y), ImVec2(mousePos.x + crossSize, mousePos.y));
		}
	}

	// if they are manipulating the canvas change to the grab cursor 
	else if (UI::getCursorMode() == CursorMode::Pan || UI::getCursorMode() == CursorMode::Rotate) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
	}
}


// methods for drawing the individual menu panels
void UI::drawTopPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(displayWidth, TopSize), ImGuiCond_Always);
	ImGui::Begin("Top Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	// add widgets
	// new canvas pop up
	if (ImGui::Button("New File")) {
		showPopup = true;
	}

	// menu to rebind the various actions that can be done with hotkeys
	// the getHotkeyString(InputAction::setRotate).c_str() is the funciton 
	// call to get the string version of the key combos
	ImGui::SameLine();
	if (ImGui::Button("Shortcuts")) {
		ImGui::OpenPopup("Shortcuts Popup");
	}

	if (ImGui::BeginPopup("Shortcuts Popup"))
	{
		// create a lamda to grab the hotkey label for each action
		auto hotkeyLabel = [this](InputAction action) {
			if (getHotkeyLabelCb) {
				return getHotkeyLabelCb(action);
			}
			return std::string{};
			};

		// create a lamda to trigger the rebind callback function for each action
		auto triggerRebind = [this](InputAction action) {
			if (startRebindCb) {
				startRebindCb(action);
			}
			// Close to avoid consuming the next key press while rebinding.
			ImGui::CloseCurrentPopup();
			};

		if (ImGui::MenuItem("Rotate", hotkeyLabel(InputAction::setRotate).c_str()))
		{
			triggerRebind(InputAction::setRotate);
		}
		if (ImGui::MenuItem("Pan", hotkeyLabel(InputAction::setPan).c_str()))
		{
			triggerRebind(InputAction::setPan);
		}
		if (ImGui::MenuItem("Draw", hotkeyLabel(InputAction::setDraw).c_str()))
		{
			triggerRebind(InputAction::setDraw);
		}
		if (ImGui::MenuItem("Fill", hotkeyLabel(InputAction::setFill).c_str())){
			triggerRebind(InputAction::setFill);
		}
		if (ImGui::MenuItem("Erase", hotkeyLabel(InputAction::setErase).c_str()))
		{
			triggerRebind(InputAction::setErase);
		}
		if (ImGui::MenuItem("Undo", hotkeyLabel(InputAction::undo).c_str()))
		{
			triggerRebind(InputAction::undo);
		}
		if (ImGui::MenuItem("Redo", hotkeyLabel(InputAction::redo).c_str()))
		{
			triggerRebind(InputAction::redo);
		}
		if (ImGui::MenuItem("Zoom In", hotkeyLabel(InputAction::setClickZoomIn).c_str()))
		{
			triggerRebind(InputAction::setClickZoomIn);
		}
		if (ImGui::MenuItem("Zoom Out", hotkeyLabel(InputAction::setClickZoomOut).c_str()))
		{
			triggerRebind(InputAction::setClickZoomOut);
		}
		if (ImGui::MenuItem("Center Canvas", hotkeyLabel(InputAction::resetView).c_str()))
		{
			triggerRebind(InputAction::resetView);
		}
		if (ImGui::MenuItem("Color Picker", hotkeyLabel(InputAction::setColor).c_str()))
		{
			triggerRebind(InputAction::setColor);
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	// Save button

	// only appears if there is a canvas
	if (ImGui::Button("Save File"))
	{
		if (canvasManager.hasActive())
		{	
			IGFD::FileDialogConfig config;

			// the  path the file explorer starts in. "." is the current active directory
			config.path = ".";

			config.fileName = canvasManager.getActive().getName();

			// ImGuiFileDialog has a built in detection for overwriting a file and makes a popup as well.
			config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

			ImGuiFileDialog::Instance()->OpenDialog(
				"SaveImageDlg",
				"Save Image",
				".png,.jpg,.ora",
				config
			);
		}
	}
	if (ImGuiFileDialog::Instance()->Display("SaveImageDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath =
				ImGuiFileDialog::Instance()->GetFilePathName();

			std::string extension =
				ImGuiFileDialog::Instance()->GetCurrentFilter();

			if (extension == ".ora")
			{
				canvasManager.saveORA(filePath);
			}

			else
			{
				canvasManager.saveToFile(filePath);
			}

			saveToRecentActivityCb(filePath);
		}

		ImGuiFileDialog::Instance()->Close();
	}

	//loading file
	ImGui::SameLine();
	if (ImGui::Button("Load File"))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			"LoadFileDlg",
			"Choose File",
			".png,.jpg,.ora"
		);
	}

	if (ImGuiFileDialog::Instance()->Display("LoadFileDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath =
				ImGuiFileDialog::Instance()->GetFilePathName();

			std::cout << "filepath" << std::endl;

			std::string extension =
				ImGuiFileDialog::Instance()->GetCurrentFilter();

			std::cout << "extension" << std::endl;

			if (extension == ".ora")
			{
				canvasManager.loadORA(filePath);
				// centering the loaded image 
				resetCanvasPositionCb();
			}
			else
			{
				canvasManager.loadFromFile(filePath);
				// centering the loaded image 
				resetCanvasPositionCb();
			}

			// save to recent activity list
			saveToRecentActivityCb(filePath);

			// if the current UI state is the start menu then change it to the main screen
			if (curState == UIState::start_menu) {curState = UIState::main_screen;}
			
		}

		ImGuiFileDialog::Instance()->Close();
	}

	// end step
	ImGui::End();
}


void UI::drawLeftPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(LeftSize, displayHeight), ImGuiCond_Always);
	ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	/////// add widgets here ///////
	// text label that displays current cursor mode
	if (getCursorMode() == CursorMode::Draw) {
		ImGui::Text("State: Draw");
	}

	else if (getCursorMode() == CursorMode::Fill){
		ImGui::Text("State: Fill");
	}

	else if (getCursorMode() == CursorMode::Erase) {
		ImGui::Text("State: Erase");
	}

	else if (getCursorMode() == CursorMode::ZoomIn) {
		ImGui::Text("State: Zoom In");
	}

	else if (getCursorMode() == CursorMode::ZoomOut) {
		ImGui::Text("State: Zoom Out");
	}

	else if (getCursorMode() == CursorMode::Rotate) {
		ImGui::Text("State: Rotate");
	}

	else if (getCursorMode() == CursorMode::Pan) {
		ImGui::Text("State: Pan");
	}

	else if (getCursorMode() == CursorMode::ColorPick) {
		ImGui::Text("State: Color Pick");
	}

	// text label that displays rebind status
	if (isWaitingForRebindCb && isWaitingForRebindCb())
	{
		ImGui::Text("Press any key...");
	}

	if (didRebindFailCb && didRebindFailCb())
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Key already bound!");
	}

	// buttons that change the current cursor mode
	if (ImGui::Button("Draw")) {
		setCursorMode(CursorMode::Draw);
	}

	if (ImGui::Button("Fill")) {
		setCursorMode(CursorMode::Fill);
	}

	if (ImGui::Button("Erase")) {
		setCursorMode(CursorMode::Erase);
	}

	if (ImGui::Button("Pan")) {
		setCursorMode(CursorMode::Pan);
	}

	if (ImGui::Button("Rotate")) {
		setCursorMode(CursorMode::Rotate);
	}

	if (ImGui::Button("Zoom In")) {
		setCursorMode(CursorMode::ZoomIn);
	}

	if (ImGui::Button("Zoom Out")) {
		setCursorMode(CursorMode::ZoomOut);
	}

	if (ImGui::Button("Color Picker")) {
		setCursorMode(CursorMode::ColorPick);
	}

	// adds a little visual split between sections
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if(canvasManager.hasActive()){
		if(ImGui::Button("Toggle Onion Skins")){
			FrameRenderer::removeOnionSkin(canvasManager.getActive());
			FrameRenderer::toggleOnionSkin();
			FrameRenderer::updateOnionSkin(canvasManager.getActive());
		}
	}
	// end step
	LeftSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, displayHeight)); // keeps its Y-value the same
	ImGui::End();
}


void UI::drawRightPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(displayWidth - RightSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(RightSize, displayHeight - TopSize), ImGuiCond_Always);
	ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	// add widgets
	// the color wheel
	ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel |
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_AlphaBar;

	ImGui::ColorPicker4("", color, flags);

	// brush size slider 
	ImGui::SliderInt("Size", &brushSize, 1, 500, "%d", ImGuiSliderFlags_Logarithmic);
	// preset brush size buttons 
	// ----- NOTE: a limited amount is added right now, will add full when UI rework is settled -----
	if (ImGui::Button("5"))  brushSize = 5;
	ImGui::SameLine();
	if (ImGui::Button("10")) brushSize = 10;
	ImGui::SameLine();
	if (ImGui::Button("100")) brushSize = 100;


	// adds a little visual split between sections
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// if there is a canvas then display the layer options
	if (canvasManager.hasActive())
	{
		// save the active canvas for later use
		ImGui::Text("file is open");
		ImGui::Text("file size is: ");
		ImGui::Text("%dx%d", canvasManager.getActive().getWidth(), canvasManager.getActive().getHeight());

		// Create the layer buttons
		if (ImGui::Button("New Layer")) {
			// increase the number of layers by 1
			canvasManager.getActive().createLayer();
		}
		// remove a layer button 

		if (ImGui::Button("Remove Layer")) {
			if (canvasManager.getActive().getNumLayers() > 2) {
				// decrease the number of layers by 1
				canvasManager.getActive().removeLayer();
			}
		}

		for (int i = 1; i < canvasManager.getActive().getNumLayers(); i++) {
			std::string buttonName = "Canvas Layer " + std::to_string(i);
			if (ImGui::Button(buttonName.c_str())) {
				canvasManager.getActive().selectLayer(i);
			}
		}
		ImGui::Text("Current Layer: %d", canvasManager.getActive().getCurLayer());

		// adds a little visual split between sections
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	// brush import system, will probably get moved when I eventually do a UI overhaul
	if (ImGui::Button("Import Brush"))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			"ChooseFileDlgKey",
			"Choose File",
			".gbr,.png,.kpp,.jbr"
		);
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			if (loadBrushFromFileCb) {
				loadBrushFromFileCb(filePath);
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// --- Displaying loaded brush options ---
	// grabs the list of loaded brushes
	static const std::vector<BrushTool> emptyBrushes;
	const std::vector<BrushTool>& brushes = getBrushListCb ? getBrushListCb() : emptyBrushes;

	// adds a button for each brush that sets it to the active one
	ImGui::Text("Loaded Brushes: ");
	for (int i = 0; i < brushes.size(); i++)
	{
		std::string buttonName = brushes[i].brushName;

		if (ImGui::Button(buttonName.c_str())) {
			if (setActiveBrushCb) {
				setActiveBrushCb(i);
			}
		}
	}

	// end step
	RightSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, displayHeight)); // keeps its Y-value the same
	ImGui::End();
}


void UI::drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, displayHeight - BotSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, BotSize), ImGuiCond_Always);
	ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	// add widgets
	// only display animation settings if there is an active canvas
	if (canvasManager.hasActive())
	{
		if(ImGui::Button("Previous Frame")){
			FrameRenderer::selectFrame(canvasManager.getActive(), -1);
		}
		ImGui::SameLine();
		if(ImGui::Button("Next Frame")){
			FrameRenderer::selectFrame(canvasManager.getActive(), 1);
		}
		
		//ImGui::SliderInt()
		if(ImGui::Button("Create Frame")){
			FrameRenderer::createFrame(canvasManager.getActive());
		}
		ImGui::SameLine();
		if(ImGui::Button("Remove Frame")){
			FrameRenderer::removeFrame(canvasManager.getActive());
		}
		if (ImGui::Button("Play")) {
			FrameRenderer::play(canvasManager.getActive());
		}
	}

	// end step
	if (ImGui::GetWindowHeight() > displayHeight - 2 * TopSize)
		BotSize = displayHeight - 2 * TopSize;
	else
		BotSize = ImGui::GetWindowHeight();
	ImGui::End();
}


void UI::drawCanvasTabs(CanvasManager& canvasManager)
{
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(LeftSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, TopSize), ImGuiCond_Always);
	ImGui::Begin("Canvas Tabs Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	// add widgets

	// --- Displaying the loaded canvases ---
	// grabs the list of loaded canvases
	const std::vector<Canvas>& canvases = canvasManager.getOpenCanvases();

	// adds a button for each canvas that sets it to the active one
	for (int i = 0; i < canvasManager.getNumCanvases(); i++)
	{
		std::string buttonName = canvases[i].getName();

		if (ImGui::Button(buttonName.c_str())) {
			canvasManager.setActiveCanvas(i);
		}

		ImGui::SameLine();
	}

	// end step
	ImGui::End();
}


// canvas size popup 
void UI::drawNewCanvasPopup(CanvasManager& canvasManager)
{
	static int temp_w = 1920;
	static int temp_h = 1080;
	static std::string temp_n = "Untitled";
	static bool isAnimation = false;

	if (showPopup) {
		ImGui::OpenPopup("New Canvas");
	}

	// specifying canvas size 
	if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::InputInt("Width:", &temp_w);
		ImGui::InputInt("Height:", &temp_h);
		ImGui::InputText("File Name:", &temp_n);
		ImGui::Checkbox("Use Animation Template", &isAnimation);

		if (isAnimation) {
			ImGui::TextWrapped("The animation template currently uses a set size of 2338 x 1653.");
			temp_w = 2338;
			temp_h = 1653;
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// if user creates a canvas, remove the popup 
		if (ImGui::Button("Create")) {

			// create the new canvas
			canvasManager.createCanvas(temp_w, temp_h, temp_n, isAnimation);

			// centering the newly created canvas 
			resetCanvasPositionCb();

			showPopup = false;
			temp_n = "Untitled";

			// if the current UI state is the start menu then change it to the main screen
			if (curState == UIState::start_menu) {curState = UIState::main_screen;}

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