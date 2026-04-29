#include <glad/glad.h>
#include "UI.h"

#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <map>
#include <tuple>
#include <unordered_map>

#include "ImGuiFileDialog.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_stdlib.h>


#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "IconsFontAwesome6.h"


std::string overwritePath;

// for the draggable buttons
struct DragState {
    float offsetY = 0.0f;
	int index = 0;
	bool notActive = false;
	int order = index;
};
static std::unordered_map<ImGuiID, DragState> dragStates;

// variables to store info for UI declared up here 
/// panel sizes
int TopSize = 0;
int BotSize = 0;
int LeftSize = 0;
int RightSize = 0;

// state initial pop up 
bool UI::showNewCanvasPopup = false;
bool UI::showSaveDialog = false;
bool UI::showOpenDialog = false;

// show panels 
static bool showPanels = true;

// reference to the window, used for input 
static GLFWwindow* windowStorage;

// the init canvas values are displayed in the text boxes
static int canvasWidth = 1920;
static int canvasHeight = 1080;

// the default starting frame
static float curFrame = 1.0f;
// RBGA values for the color wheel 
static float color[4] = { .0f, .0f, .0f, 1.0f };

// RGBA values for primary and secondary color swatches 
static ImVec4 primary_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
static ImVec4 secondary_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

// determines which color swatch the color wheel is bound to 
static bool editing_primary = true;

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

// struct for storing one loaded preview texture 
struct PreviewImage
{
	GLuint texture = 0; // texture ID
	int width = 0;
	int height = 0;
};

// cache to prevent image reloading every frame 
std::unordered_map<std::string, PreviewImage> previewCache;


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

void UI::bindDefaultFolderPathCallback(getDefaultFolderPathCallback getPathCb, setDefaultFolderPathCallback setPathCb)
{
	getDefaultFolderPathCb = std::move(getPathCb);
	setDefaultFolderPathCb = std::move(setPathCb);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);
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

PreviewImage* getPreview(const std::string& path) {
	// check if we have already loaded this image, stop if we have
	auto imageThumbnail = previewCache.find(path);
	if (imageThumbnail != previewCache.end())
		// reusing stored image
		return &imageThumbnail->second;

	// if we haven't used it before, load it from file
	PreviewImage preview;

	bool previewLoaded = LoadTextureFromFile(path.c_str(), &preview.texture, &preview.width, &preview.height);

	if (!previewLoaded) {
		// fuckin explode if it didn't work
		return nullptr;
	}

	// store in cache to prevent reloading constantly	
	previewCache[path] = preview;
	return &previewCache[path];
}


// note: not all options here return a value. 
Color UI::getColor()
{
	const CursorMode mode = getCursorMode();

	if (mode == CursorMode::Draw || mode == CursorMode::Fill)
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
	// picking which one to update 
	ImVec4* active_color = editing_primary ? &primary_color : &secondary_color;

	// having to set the color through active_color to account for color swatch implementation
	active_color->x = static_cast<float>(currentPixelColor.r) / 255.0f;
	active_color->y = static_cast<float>(currentPixelColor.g) / 255.0f;
	active_color->z = static_cast<float>(currentPixelColor.b) / 255.0f;
	active_color->w = static_cast<float>(currentPixelColor.a) / 255.0f;

	color[0] = active_color->x;
	color[1] = active_color->y;
	color[2] = active_color->z;
	color[3] = active_color->w;
}


// cursor mode get and set
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


// ----- UI initialization -----
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

	// ---- premade stuff to load fonts ----
	ImGuiIO& io = ImGui::GetIO();

	// Load default font
	io.Fonts->AddFontDefault();

	// Setup icon merge
	ImFontConfig config;
	config.MergeMode = true;
	config.PixelSnapH = true;

	// Define icon range
	static const ImWchar ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	// loading font and setting size 
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		"assets/fa-solid-900.ttf", // possible pain point for Gavin
		14.0f,
		&config,
		ranges
	);

	if (!font) {
		printf("Font load failed!\n");
	}

	// Build atlas ONCE
	io.Fonts->Build();



	// storing window for user input 
	windowStorage = window;

	// set initial visibility of UI elements
	for (UIElement e : elements) {
		elementVisibility[e] = true;
	}
}


// NOTE: called in render loop 
void UI::drawUI(CanvasManager& canvasManager, FrameRenderer frameRenderer)
{
	// start ImGui frame before adding widgets 
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMainMenu(canvasManager);

	// changing hard set display size to allow for docking tabs 
	// grab the window display size
	ImGuiIO& io = ImGui::GetIO();
	displayWidth = io.DisplaySize.x;
	displayHeight = io.DisplaySize.y;

	// compute the panel sizes
	if (TopSize == 0) { TopSize = 20; }
	if (BotSize == 0) { BotSize = static_cast<int>(0.25 * displayHeight); }
	if (LeftSize == 0) { LeftSize = static_cast<int>(0.1 * displayWidth); }
	if (RightSize == 0) { RightSize = static_cast<int>(0.1 * displayWidth); }

	// ----- Call the various popup draws so they get drawn when needed -----
	drawNewCanvasPopup(canvasManager); 	// draw the new canvas pop up
	drawSettingsPopup(canvasManager); 	// draw the settings menu pop up

	// ----- Cursor Customization -----
	drawCustomCursor(canvasManager);

	// draw ui based on the UI's current state
	if (curState == UIState::start_menu) { drawStartScreen(canvasManager); }
	else if (curState == UIState::main_screen) { drawMainScreen(canvasManager, frameRenderer); }

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

	if (ImGui::Button("Create File" " " ICON_FA_FILE "" ICON_FA_PLUS, buttonSize))
	{
		UI::showNewCanvasPopup = true;
	}

	ImGui::Dummy(ImVec2(0, 10)); 			// creates some vertical space
	ImGui::SetCursorPosX(centerX); 			// center the button

	if (ImGui::Button("Open File" " " ICON_FA_FOLDER, buttonSize))
	{
		IGFD::FileDialogConfig config;
		config.path = ".";

		if (getDefaultFolderPathCb) {
			config.path = getDefaultFolderPathCb();
		}

		ImGuiFileDialog::Instance()->OpenDialog(
			"LoadFileDlg",
			"Choose File",
			".png,.jpg,.ora",
			config
		);
	}

	if (ImGuiFileDialog::Instance()->Display("LoadFileDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

			std::string extension = ImGuiFileDialog::Instance()->GetCurrentFilter();

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
			if (curState == UIState::start_menu) { curState = UIState::main_screen; }

		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("LoadFileAnim"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

			std::string extension = ImGuiFileDialog::Instance()->GetCurrentFilter();

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
			if (curState == UIState::start_menu) { curState = UIState::main_screen; }

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

			// putting button and image preview together	
			ImGui::BeginGroup();

			// if the button is clicked then load in the file
			std::string extension = filePath.substr(filePath.find_last_of('.'));

			// need to figure something out for ora files 
			if (extension != ".ora") {
				PreviewImage* preview = getPreview(filePath);

				if (preview && preview->texture) {
					float maxSize = 200.0f;
					float ratio = (float)preview->width / (float)preview->height;

					ImVec2 size;
					if (ratio > 1.0f)
						size = ImVec2(maxSize, maxSize / ratio);
					else
						size = ImVec2(maxSize * ratio, maxSize);
					ImGui::Image((ImTextureID)(intptr_t)preview->texture, size);
				}
			}

			if (ImGui::Button(fileName.c_str(), ImVec2(150, 0))) {
				if (extension == ".ora")
					canvasManager.loadORA(filePath);
				else
					canvasManager.loadFromFile(filePath);
				// centering the loaded image 
				resetCanvasPositionCb();

				// if the current UI state is the start menu then change it to the main screen
				if (curState == UIState::start_menu) { curState = UIState::main_screen; }
			}

			ImGui::EndGroup();

		}
	}

	// end step
	ImGui::End();
}

void UI::drawMainScreen(CanvasManager& canvasManager, FrameRenderer frameRenderer)
{
	// -- user input to hide UI panels --
	// only allow this if the canvas creation popup is not active 
	if (!ImGui::IsPopupOpen("New")) {
		if (glfwGetKey(windowStorage, GLFW_KEY_TAB) == GLFW_PRESS && glfwGetTime() - lastFrame >= 0.2) {
			showPanels = !showPanels;
			lastFrame = glfwGetTime();
		}
	}

	// draw the UI elements only if showPanels is true
	// draw the three main screen panels
	if (showPanels) {
		// if in default mode draw only the default panels
		if (uiMode == UIMode::Default) {
			drawLeftPanel(canvasManager);
			drawRightPanel(canvasManager);
			drawBottomPanel(canvasManager, frameRenderer);
			// canvas tab panel shown only if more than 1 canvas is open
			if (canvasManager.getNumCanvases() > 0) { drawCanvasTabs(canvasManager); }

		}
		// if in modular mode then draw the individual elements based on their visibility
		else if (uiMode == UIMode::Modular) {
			if (elementVisibility[UIElement::colorWheel]) { drawColorWindow(canvasManager); }
			if (elementVisibility[UIElement::brushSizeSlider]) { drawBrushSizeWindow(canvasManager); }
			if (elementVisibility[UIElement::brushSelection]) { drawBrushesWindow(canvasManager); }
			if (elementVisibility[UIElement::cursorModeButtons]) { drawCursorModesWindow(canvasManager); }
			if (elementVisibility[UIElement::animationTimeline]) { drawTimelineWindow(canvasManager); }
			if (elementVisibility[UIElement::layers]) { drawLayersWindow(canvasManager); }
		}
	}

	if (FrameRenderer::inputBlocked) {
		drawBlockPanel(canvasManager);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

		if (brushSize >= 5) {
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
		// the default cross cursor used for smaller brush sizes 
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

// ----- drawing the individual menu panels -----
void UI::drawLeftPanel(CanvasManager& canvasManager) {
	// initialize the panel
	ImGui::SetNextWindowPos(ImVec2(0, TopSize), ImGuiCond_Always); // line that needs to change to make it fit MainMenuBar
	ImGui::SetNextWindowSize(ImVec2(LeftSize, displayHeight - TopSize), ImGuiCond_Always);
	ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	renderCursorModes(canvasManager);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// color set section
	// need active color here so that it is updated every frame
	ImVec4* active_color = editing_primary ? &primary_color : &secondary_color;
	renderColorSet(canvasManager, active_color);

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

	// ----- Color wheel ------
	// Determine which pointer to pass to the picker
	ImVec4* active_color = editing_primary ? &primary_color : &secondary_color;

	renderColorWheel(canvasManager, active_color);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	// comment that says Mori Calliope 
	// brush size slider and presets
	renderBrushSize(canvasManager);

	// adds a little visual split between sections
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// if there is a canvas then display the layer options
	bool removeLayer = false;
	if (canvasManager.hasActive())
	{
		renderLayerInfo(canvasManager);
		// adds a little visual split between sections
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	// brush import system
	renderBrushImports(canvasManager);

	if(canvasManager.hasActive()){
		std::vector<std::tuple<bool, float, int>> buttons;
		int numDragStates = 0;
		for (int i = 1; i < canvasManager.getActive().getNumLayers(); i++) {
			std::string buttonName = "Canvas Layer " + std::to_string(i);
			auto button = drawDraggableButton(canvasManager, buttonName.c_str(), i);
			buttons.push_back(button);
			numDragStates++;
		}
		for (int i = 0; i < buttons.size(); i++){
			if(std::get<0>(buttons[i])){
				canvasManager.getActive().selectLayer(std::get<2>(buttons[i]));
			}
		}
		if(removeLayer){
			canvasManager.getActive().removeLayer();
			int maxIndex = -1;
			ImGuiID maxId = 0;
			for (auto& [id, state] : dragStates)
			{
				if (state.index > maxIndex)
				{
					maxIndex = state.index;
					maxId = id;
				}
			}
			dragStates.erase(maxId);
		}
	}
	
	// end step
	RightSize = ImGui::GetWindowWidth();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetWindowSize(ImVec2(size.x, displayHeight)); // keeps its Y-value the same
	ImGui::End();
}

void UI::drawBottomPanel(CanvasManager& canvasManager, FrameRenderer frameRenderer) {
	if (canvasManager.getActive().isAnimation()) {
		ImGui::SetNextWindowPos(ImVec2(LeftSize, displayHeight - BotSize), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, BotSize), ImGuiCond_Always);
		ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	} else {
		float tempSize = displayHeight * 0.05f;
		ImGui::SetNextWindowPos(ImVec2(LeftSize, displayHeight - tempSize), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, tempSize), ImGuiCond_Always);
		ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	}

	// only draw if there is an animations
	if (canvasManager.hasActive() && canvasManager.getActive().isAnimation()) {
		// drawing the actual timeline 
		renderTimelineControls(canvasManager);
		renderTimeline(canvasManager);
	}

	// end step
	if (canvasManager.getActive().isAnimation()) {
		BotSize = ImGui::GetWindowHeight();
	}
	ImGui::End();
}

void UI::drawBlockPanel(CanvasManager& canvasManager) {
	ImGui::SetNextWindowPos(ImVec2(LeftSize, TopSize));
	ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, displayHeight - TopSize - BotSize));
	ImGui::Begin("Blocker", nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground);
	ImGui::End();
}

void UI::drawCanvasTabs(CanvasManager& canvasManager)
{
	if (canvasManager.getNumCanvases() <= 0)
		return;

	ImGui::SetNextWindowPos(ImVec2(LeftSize, TopSize), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(displayWidth - LeftSize - RightSize, 35), ImGuiCond_Always);
	ImGui::Begin("Canvas Tabs Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	if (ImGui::BeginTabBar("CanvasTabBar", ImGuiTabBarFlags_AutoSelectNewTabs))
	{
		const std::vector<Canvas>& canvases = canvasManager.getOpenCanvases();

		for (int i = 0; i < canvasManager.getNumCanvases(); i++)
		{
			bool open = true;
			const Canvas& c = canvasManager.getOpenCanvases()[i];
			std::string label = c.getName() + "##canvas" + std::to_string(i);

			bool tabvisible = ImGui::BeginTabItem(label.c_str(), &open);

			if (ImGui::IsItemActivated() && canvasManager.getActiveCanvasIndex() != i)
				canvasManager.setActiveCanvas(i);

			if (tabvisible)
				ImGui::EndTabItem();

			if (c.isDirty)
			{
				ImVec2 tabMin = ImGui::GetItemRectMin();
				ImVec2 tabMax = ImGui::GetItemRectMax();

				float textWidth = ImGui::CalcTextSize(c.getName().c_str()).x;

				ImVec2 pos = ImVec2(tabMin.x + 6.0f + textWidth, tabMin.y + 2.0f);

				ImGui::GetWindowDrawList()->AddText(pos, IM_COL32(255, 255, 255, 200), "*");
			}

			if (!open)
			{
				if (canvasManager.getOpenCanvases()[i].isDirty)
				{
					pendingCloseIndex = i;
					showCloseConfirm = true;
				}
				else
					canvasManager.closeCanvas(i);
			}
		}

		ImGui::EndTabBar();
	}

	if (showCloseConfirm)
		ImGui::OpenPopup("Close Canvas?");

	const std::vector<Canvas>& canvases = canvasManager.getOpenCanvases();

	if (ImGui::BeginPopupModal("Close Canvas?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (pendingCloseIndex >= 0 && pendingCloseIndex < canvasManager.getNumCanvases())
			ImGui::Text("Close \"%s\"?", canvases[pendingCloseIndex].getName().c_str());

		ImGui::Spacing();

		if (ImGui::Button("Save and Close"))
		{
			canvasManager.setActiveCanvas(pendingCloseIndex);
			IGFD::FileDialogConfig config;

			if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
				else config.path = ".";
			
			config.fileName = canvasManager.getActive().getName();
			config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

			const char* filters = canvasManager.getActive().isAnimation()
				? ".png,.jpg"
				: ".png,.jpg,.ora";

			ImGuiFileDialog::Instance()->OpenDialog("SaveBeforeCloseDlg", "Save Image", filters, config);
			showCloseConfirm = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Close Without Saving"))
		{
			canvasManager.closeCanvas(pendingCloseIndex);
			pendingCloseIndex = -1;
			showCloseConfirm = false;
			ImGui::CloseCurrentPopup();

			if (pendingAppClose)
				requestAppClose(canvasManager);
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			pendingCloseIndex = -1;
			showCloseConfirm = false;
			pendingAppClose = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGuiFileDialog::Instance()->Display("SaveBeforeCloseDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			canvasManager.setActiveCanvas(pendingCloseIndex);
			//int* meta = FrameRenderer::readMetaData();
			//canvasManager.getActive().setPixels(FrameRenderer::frames[FrameRenderer::curFrame - 1]);

			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string extension = ImGuiFileDialog::Instance()->GetCurrentFilter();

			if (canvasManager.getActive().isAnimation() && (extension == ".png" || extension == ".jpg"))
				FrameRenderer::saveAnimation(filePath, canvasManager.getActive());
			else if (extension == ".ora")
				canvasManager.saveORA(filePath);
			else
				canvasManager.saveToFile(filePath);

			canvasManager.getActive().isDirty = false;
			canvasManager.closeCanvas(pendingCloseIndex);

			if (pendingAppClose)
				requestAppClose(canvasManager);
		}

		pendingCloseIndex = -1;
		showCloseConfirm = false;
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::End();
}


// ---- popups which are drawn conditionally -----
void UI::drawNewCanvasPopup(CanvasManager& canvasManager)
{
	static int temp_w = 1920;
	static int temp_h = 1080;
	// add paper layer color here 
	static std::string temp_n = "Illustration";
	static std::string temp_n_a = "Animation";
	// want to also add canvas size presets as a map accessed from a combo 
	static bool animTemplate = false;

	static ImVec4 paperColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // default white 
	static bool changePaperColor = false;

	if (UI::showNewCanvasPopup) {
		ImGui::OpenPopup("New");
	}

	// specifying canvas size 
	if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		// containing the creation buttons within a tab bar 
		// different tabs for drawing and for animation
		ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("New", tabBarFlags)) {
			if (ImGui::BeginTabItem("Illustration")) {

				// trying to implement changing paper color upon canvas creation
				if (ImGui::Button("Set Paper Color")) {
					changePaperColor = !changePaperColor;
				}
				if (changePaperColor) {
					ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs;
					ImGui::SetNextItemWidth(180.0f);
					ImGui::ColorPicker4("##papercolorpicker", (float*)&paperColor, flags);
				}

				// setting up combo box for illustration presets 
				// presets as str and tuple of two ints 
				const std::map<std::string, std::tuple<int, int>> canvasSizes = {
					{"Custom", {temp_w, temp_h}},
					{"A4", {2894, 4093}}, {"A5", {2039, 2894}}, {"A6", {1447, 2039}},
					{"B4", {3541, 5016}}, {"B5", {3541, 5016}}, {"B6", {1764, 2508}},
					{"Postcard", {1378, 2039}}
				};
				// initally selected combo box value
				static std::string selectedPreset = canvasSizes.begin()->first;
				if (ImGui::InputInt("Width:", &temp_w)) { selectedPreset = "Custom"; }
				if (ImGui::InputInt("Height:", &temp_h)) { selectedPreset = "Custom"; }
				ImGui::InputText("File Name:", &temp_n);
				// have to use begin combo API because of the choice of a map 
				if (ImGui::BeginCombo("Presets", selectedPreset.c_str())) {
					for (auto const& [preset, sizes] : canvasSizes) {
						bool isSelected = (selectedPreset == preset);
						if (ImGui::Selectable(preset.c_str(), isSelected)) {
							// updating combo box selection 
							selectedPreset = preset;
							if (preset != "Custom") {
								auto& myTuple = canvasSizes.at(preset.c_str());
								temp_w = std::get<0>(myTuple);
								temp_h = std::get<1>(myTuple);
							}
						}
						if (isSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// button to swap dimensions 
				if (ImGui::Button("Swap")) {
					static int swapTemp;
					swapTemp = temp_w;
					temp_w = temp_h;
					temp_h = swapTemp;
				}

				// if user creates a canvas, remove the popup 
				if (ImGui::Button("Create")) {

					// create the new canvas
					canvasManager.createCanvas(temp_w, temp_h, temp_n, false, false, paperColor);

					// centering the newly created canvas 
					resetCanvasPositionCb();

					UI::showNewCanvasPopup = false;
					temp_n = "Illustration";

					// if the current UI state is the start menu then change it to the main screen
					if (curState == UIState::start_menu) { curState = UIState::main_screen; }

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel")) {
					UI::showNewCanvasPopup = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Animation")) {
				// input params animation creation
				ImGui::InputInt("Width:", &temp_w);
				ImGui::InputInt("Height:", &temp_h);
				ImGui::InputText("File Name:", &temp_n_a);

				// checkbox for using anim template
				ImGui::Checkbox("Use Animation Template", &animTemplate);
				if (animTemplate) {
					ImGui::TextWrapped("The animation template currently uses a set size of 2338 x 1653.");
					temp_w = 2338;
					temp_h = 1653;
				}

				// if user creates a canvas, remove the popup 
				if (ImGui::Button("Create")) {

					// create the new canvas
					canvasManager.createCanvas(temp_w, temp_h, temp_n_a, true, animTemplate);

					// centering the newly created canvas 
					resetCanvasPositionCb();

					UI::showNewCanvasPopup = false;
					temp_n_a = "Animation";

					// if the current UI state is the start menu then change it to the main screen
					if (curState == UIState::start_menu) { curState = UIState::main_screen; }

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel")) {
					UI::showNewCanvasPopup = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndTabItem();
			}
			// 
			ImGui::EndTabBar();
		}

		ImGui::EndPopup();
	}

}

void UI::drawSettingsPopup(CanvasManager& canvasManager) {
	static int settingsSection = 0;

	if (showSettingsPopup) {
		ImGui::OpenPopup("Settings");
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
	}

	if (ImGui::BeginPopupModal("Settings", nullptr, ImGuiWindowFlags_NoResize)) {
		// Create a child region that will take all remaining vertical space
		float windowHeight = ImGui::GetContentRegionAvail().y - 50; // leave ~50px for Close button
		ImGui::BeginChild("SettingsContent", ImVec2(0, windowHeight), false);

		ImGui::Columns(2, nullptr, true);
		ImGui::SetColumnWidth(0, 150);

		// Left column: categories
		if (ImGui::Button("Folder Settings##settings_folders")) settingsSection = 0;
		if (ImGui::Button("Shortcut Settings##settings_shortcuts")) settingsSection = 1;
		if (ImGui::Button("Canvas Settings##settings_canvases")) settingsSection = 2;
		if (ImGui::Button("MockUp Settings##settings_mockup")) settingsSection = 3;

		ImGui::NextColumn();

		// Right column: content
		if (settingsSection == 0) {
			ImGui::Text("Folder adjacent settings: Saving / Loading / Imports");
			// default folder path stuff	
			ImGui::SeparatorText("Default Folder Path");
			ImGui::TextWrapped("The default path used for loading and saving files.");
			ImGui::Spacing();

			std::string currentPath = getDefaultFolderPathCb ? getDefaultFolderPathCb() : std::string("Not Set");
			ImGui::TextWrapped("Current Default Path: %s", currentPath.c_str());
			ImGui::Spacing();

			if (ImGui::Button("Set Default Folder Path")) {
				IGFD::FileDialogConfig config;
				config.path = ".";

				if (getDefaultFolderPathCb) {
					config.path = getDefaultFolderPathCb();
				}

				ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, config);

			}

			if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					// Get the relative path or folder name
					std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

					// Action to perform after selection
					std::cout << filePath << std::endl;
					setDefaultFolderPathCb(filePath);
				}
				ImGuiFileDialog::Instance()->Close();
			}

		}
		else if (settingsSection == 1) {
			// text label that displays rebind status
			ImGui::Text("Click a button, then press a key to rebind.");
			if (isWaitingForRebindCb && isWaitingForRebindCb()) {
				ImGui::Text("Press any key...");
			}

			auto hotkeyLabel = [this](InputAction action) {
				return getHotkeyLabelCb ? getHotkeyLabelCb(action) : std::string{};
				};
			auto triggerRebind = [this](InputAction action) {
				if (startRebindCb) startRebindCb(action);
				};
			auto ShortcutRow = [&](const char* name, InputAction action) {
				ImGui::Text("%s", name);
				ImGui::SameLine(180);

				if (ImGui::Button((hotkeyLabel(action) + "##" + name).c_str()))
					triggerRebind(action);

				// buttons for unbinding hotkeys
				ImGui::SameLine(350);
				std::string unbindLabel = std::string("x##unbind_") + name;
				ImGui::BeginDisabled(hotkeyLabel(action) == "Unbound");
				if (ImGui::SmallButton(unbindLabel.c_str()))
					InputManager::unbindAction(action);
				ImGui::EndDisabled();
			};

			ImGui::SeparatorText("Document Shortcuts:");
			ShortcutRow("New File", InputAction::newFile);
			ShortcutRow("Close File", InputAction::closeCanvas);
			ShortcutRow("Save File", InputAction::saveFile);
			ShortcutRow("Open File", InputAction::openFile);

			ImGui::SeparatorText("Drawing Shortcuts:");
			ShortcutRow("Draw", InputAction::setDraw);
			ShortcutRow("Erase", InputAction::setErase);
			ShortcutRow("Fill", InputAction::setFill);
			ShortcutRow("Color Picker", InputAction::setColor);
			ShortcutRow("Undo", InputAction::undo);
			ShortcutRow("Redo", InputAction::redo);

			ImGui::SeparatorText("Navigation Shortcuts:");
			ShortcutRow("Rotate", InputAction::setRotate);
			ShortcutRow("Pan", InputAction::setPan);
			ShortcutRow("Zoom In", InputAction::setClickZoomIn);
			ShortcutRow("Zoom Out", InputAction::setClickZoomOut);
			ShortcutRow("Center Canvas", InputAction::resetView);

			ImGui::SeparatorText("Animation Shortcuts:");
			ShortcutRow("Previous Frame", InputAction::prevFrame);
			ShortcutRow("Next Frame", InputAction::nextFrame);
			ShortcutRow("Create Frame", InputAction::newFrame);
			ShortcutRow("Remove Frame", InputAction::removeFrame);
			ShortcutRow("Onion Skin Toggle", InputAction::onionSkinToggle);
			ShortcutRow("Play animation", InputAction::quickPlay);
		}

		else if (settingsSection == 2) {
			// only show canvas settings if there is an active canvas
			if (canvasManager.hasActive()) {
				bool isAnimTemplate = canvasManager.getActive().isUsingAnimTemplate();
				ImGui::Text("Canvas settings: Canvas behavior, animation settings, etc.");
				static bool changePaperColor = false;
				if (isAnimTemplate) {
					ImGui::TextWrapped("Some settings are disabled for canvases using the animation template.");
				}

				if (ImGui::Button("Change Paper Color")) {
					changePaperColor = true;
				}
				if (changePaperColor && !isAnimTemplate) {
					ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs;
					static ImVec4 paperColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // default white
					ImGui::SetNextItemWidth(180.0f);
					ImGui::ColorPicker4("##papercolorpicker", (float*)&paperColor, flags);
					if (ImGui::Button("Apply")) {
						canvasManager.setPaperColor(paperColor);
						changePaperColor = false;
						canvasManager.getActive().isDirty = true;
					}
				}
			}
			else
				ImGui::Text("Canvas settings will appear here when a canvas is open.");

		}

		else if (settingsSection == 3) {
			if (curState != UIState::start_menu) {
				if (ImGui::Button("Return to Main Menu")) {
					curState = UIState::start_menu;
				}
			}
			else
				ImGui::Text("Already on Main Menu");
		}
		ImGui::Columns(1);
		ImGui::EndChild();

		// Separator and Close button at the bottom
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Close", ImVec2(-1, 0))) { // -1 width = full width
			showSettingsPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


// ----- main menu bar for file, animation, settings, etc... -----
//       nested menus 
void UI::drawMainMenu(CanvasManager& canvasManager) {
	// starting the overall main menu bar
	// testing the main menu implementation

	// for saving file hotkey
	if (showSaveDialog && canvasManager.hasActive())
	{
		showSaveDialog = false;
		IGFD::FileDialogConfig config;
		if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
		else config.path = ".";
		config.fileName = canvasManager.getActive().getName();
		config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

		if (canvasManager.getActive().isAnimation())
			ImGuiFileDialog::Instance()->OpenDialog("SaveImageAnm", "Save Image", ".png,.jpg", config);
		
		else
			ImGuiFileDialog::Instance()->OpenDialog("SaveImageDlg", "Save Image", ".png,.jpg,.ora", config);
	}

	// for opening file hotkey
	if (showOpenDialog)
	{
		showOpenDialog = false;
		IGFD::FileDialogConfig config;
		if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
		else config.path = ".";
		ImGuiFileDialog::Instance()->OpenDialog(
			"LoadFileDlg",
			"Choose File",
			".png,.jpg,.ora",
			config
		);
	}

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New...", "Ctrl+N")) {
				UI::showNewCanvasPopup = true;
			}
			if (ImGui::MenuItem("Save...", "Ctrl+S", false, canvasManager.hasActive())) {
				IGFD::FileDialogConfig config;

				// the  path the file explorer starts in. "." is the current active directory
				if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
				else config.path = ".";

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
			//saving animation
			if (ImGui::MenuItem("Save Animation", "", false, canvasManager.hasActive() && canvasManager.getActive().isAnimation()))
			{
				IGFD::FileDialogConfig config;

				// the  path the file explorer starts in. "." is the current active directory
				if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
				else config.path = ".";

				config.fileName = canvasManager.getActive().getName();

				// ImGuiFileDialog has a built in detection for overwriting a file and makes a popup as well.
				config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

				ImGuiFileDialog::Instance()->OpenDialog(
					"SaveImageAnm",
					"Save Animation",
					".png,.jpg",
					config
				);
			}

			if (ImGui::MenuItem("Open...", "Ctrl+O")) {
				IGFD::FileDialogConfig config;

				// the  path the file explorer starts in. "." is the current active directory
				if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
				else config.path = ".";

				ImGuiFileDialog::Instance()->OpenDialog(
					"LoadFileDlg",
					"Choose File",
					".png, .jpg, .ora",
					config
				);
			}
			
			if (ImGui::MenuItem("Open Animation...", "")) {
				IGFD::FileDialogConfig config;

				// the  path the file explorer starts in. "." is the current active directory
				if (getDefaultFolderPathCb) config.path = getDefaultFolderPathCb();
				else config.path = ".";

				ImGuiFileDialog::Instance()->OpenDialog(
					"LoadAnimDlg",
					"Choose File",
					nullptr,
					config
				);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);

			// toggle between default and modular UI modes
			if (ImGui::MenuItem("Default UI", nullptr, uiMode == UIMode::Default)) {
				uiMode = UIMode::Default;
			}
			if (ImGui::MenuItem("Modular UI", nullptr, uiMode == UIMode::Modular)) {
				uiMode = UIMode::Modular;
			}

			// set visibility of individual elements
			if (ImGui::MenuItem("Color Wheel", nullptr, elementVisibility[UIElement::colorWheel])) {
				elementVisibility[UIElement::colorWheel] = !elementVisibility[UIElement::colorWheel];
			}
			if (ImGui::MenuItem("Brush Size", nullptr, elementVisibility[UIElement::brushSizeSlider])) {
				elementVisibility[UIElement::brushSizeSlider] = !elementVisibility[UIElement::brushSizeSlider];
			}
			if (ImGui::MenuItem("Brush Selection", nullptr, elementVisibility[UIElement::brushSelection])) {
				elementVisibility[UIElement::brushSelection] = !elementVisibility[UIElement::brushSelection];
			}
			if (ImGui::MenuItem("Cursor Mode Buttons", nullptr, elementVisibility[UIElement::cursorModeButtons])) {
				elementVisibility[UIElement::cursorModeButtons] = !elementVisibility[UIElement::cursorModeButtons];
			}
			if (ImGui::MenuItem("Animation Timeline", nullptr, elementVisibility[UIElement::animationTimeline])) {
				elementVisibility[UIElement::animationTimeline] = !elementVisibility[UIElement::animationTimeline];
			}
			if (ImGui::MenuItem("Layers", nullptr, elementVisibility[UIElement::layers])) {
				elementVisibility[UIElement::layers] = !elementVisibility[UIElement::layers];
			}

			ImGui::PopItemFlag();
			ImGui::EndMenu();
		}
		float width = ImGui::CalcTextSize("Settings").x;
		float avail = ImGui::GetContentRegionAvail().x;

		ImGui::SameLine(ImGui::GetCursorPosX() + avail - width - 15);
		if (ImGui::MenuItem("Settings"))
		{
			showSettingsPopup = true;
		}

		ImGui::EndMainMenuBar();
	}

	// file dialogs need to be placed outside of the main menu bar 
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

			// changes the name of canvasTab if saved
			size_t lastSlash = filePath.find_last_of("/\\");
			size_t lastDot = filePath.find_last_of('.');
			std::string newName = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
			canvasManager.getActive().setName(newName);

			saveToRecentActivityCb(filePath);
			canvasManager.getActive().isDirty = false;
		}

		ImGuiFileDialog::Instance()->Close();
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

			// if the current UI state is the start menu then change it to the main screen
			if (curState == UIState::start_menu) { curState = UIState::main_screen; }
			saveToRecentActivityCb(filePath);

		}

		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("SaveImageAnm"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath =
				ImGuiFileDialog::Instance()->GetFilePathName();

			std::string extension =
				ImGuiFileDialog::Instance()->GetCurrentFilter();

			FrameRenderer::saveAnimation(filePath, canvasManager.getActive());
			canvasManager.getActive().isDirty = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("LoadAnimDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string folder = ImGuiFileDialog::Instance()->GetCurrentPath();

			canvasManager.loadAnimation(folder);
			
			// if the current UI state is the start menu then change it to the main screen
			if (curState == UIState::start_menu) { curState = UIState::main_screen; }
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

// ----- drawing individual windows for modular UI -----

void UI::drawColorWindow(CanvasManager& canvasManager) {
	// name our window 
	ImGui::Begin("Color");
	ImGui::Text("Color");
	// ----- Color wheel ------
	// Determine which pointer to pass to the picker
	ImVec4* active_color = editing_primary ? &primary_color : &secondary_color;

	renderColorWheel(canvasManager, active_color);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// for modular UI, checkbox to decide if palette is rendered 
	static bool showPalette = true;
	ImGui::Checkbox("Show Color Set", &showPalette);
	if (showPalette) {
		renderColorSet(canvasManager, active_color);
	}
	ImGui::End();
}

void UI::drawBrushSizeWindow(CanvasManager& canvasManager) {
	ImGui::Begin("Brush Size");
	renderBrushSize(canvasManager);
	ImGui::End();
}

void UI::drawLayersWindow(CanvasManager& canvasManager) {


	// if there is a canvas then display the layer options
	if (canvasManager.hasActive())
	{
		ImGui::Begin("Layers");
		renderLayerInfo(canvasManager);
		ImGui::End();
	}
}

void UI::drawBrushesWindow(CanvasManager& canvasManager) {
	ImGui::Begin("Brushes");
	renderBrushImports(canvasManager);
	ImGui::End();
}

void UI::drawCursorModesWindow(CanvasManager& canvasManager) {
	ImGui::Begin("CursorModes");
	renderCursorModes(canvasManager);
	ImGui::End();
}

void UI::drawTimelineWindow(CanvasManager& canvasManager) {
	// only draw if there is an animations
	if (!canvasManager.hasActive() || !canvasManager.getActive().isAnimation())
		return;

	ImGui::Begin("Timeline");
	renderTimelineControls(canvasManager);
	renderTimeline(canvasManager);
	ImGui::End();
}


// ---- separate methods to handle rendering all components ----- 
//      done to unify the components across windows and panels
void UI::renderTimeline(CanvasManager& canvasManager) {
	Canvas& activeCanvas = canvasManager.getActive();

	// dimensions for our animation cels
	const float k_cellWidth = 20.0f;
	const float k_cellHeight = 20.0f;
	int totalFrames = FrameRenderer::getNumFrames();
	int currentFrame = FrameRenderer::getCurFrame();
	int numLayers = activeCanvas.getNumLayers();
	int currentLayerIdx = activeCanvas.getCurLayer();

	// timeline flags
	static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX |
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("TimelineGrid", totalFrames + 1, flags)) {

		ImGui::TableSetupScrollFreeze(1, 1);

		// column 0 - contains timeline title 
		ImGui::TableSetupColumn("Animation Layers", ImGuiTableColumnFlags_WidthFixed, 150.0f);

		// remaining columns - frame numbers 
		for (int n = 1; n <= totalFrames; n++) {
			// Use a pointer to a string or a static buffer to ensure the label persists during the call
			std::string frameLabel = std::to_string(n);
			ImGui::TableSetupColumn(frameLabel.c_str(), ImGuiTableColumnFlags_WidthFixed, k_cellWidth);
		}

		ImGui::TableHeadersRow();

		// iterating through layers starting at 1 
		for (int row = 1; row < numLayers; row++) {
			ImGui::TableNextRow(ImGuiTableRowFlags_None, k_cellHeight);

			// column 0 - layer information
			ImGui::TableSetColumnIndex(0);
			bool isLayerSelected = (row == currentLayerIdx);

			std::string label = ICON_FA_LAYER_GROUP " Layer " + std::to_string(row);
			if (ImGui::Selectable(label.c_str(), isLayerSelected, ImGuiSelectableFlags_None, ImVec2(0.0f, k_cellHeight))) {
				activeCanvas.selectLayer(row);
			}

			// remaining columns - animation cels
			for (int frame = 1; frame <= totalFrames; frame++) {
				ImGui::TableSetColumnIndex(frame);
				ImGui::PushID(row * 1000 + frame); // unique ID per cell

				bool isCurrentFrame = (frame == currentFrame);

				// colors:
				// 1. red if its the selected cel
				// 2. darker if its the selected layer
				// 3. grey for all others
				ImU32 cellBgColor = IM_COL32(50, 50, 50, 255);
				if (isCurrentFrame) cellBgColor = IM_COL32(255, 0, 0, 80);
				else if (isLayerSelected) cellBgColor = IM_COL32(80, 80, 80, 255);

				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);

				// interaction: able to select frame and layer simultaneously 
				// EX: selecting layer y at frame x 
				if (ImGui::Selectable("##cell", isCurrentFrame && isLayerSelected, ImGuiSelectableFlags_None, ImVec2(k_cellWidth, k_cellHeight))) {
					// update Layer
					activeCanvas.selectLayer(row);
					// update Frame
					int offset = frame - currentFrame;
					if (offset != 0) {
						FrameRenderer::selectFrame(activeCanvas, offset);
					}
				}
				ImGui::PopID();
			}
		}
		ImGui::EndTable();
	}
}

void UI::renderTimelineControls(CanvasManager& canvasManager) {
	if (ImGui::Button(ICON_FA_PLUS)) {
		FrameRenderer::createFrame(canvasManager.getActive());
	}
	ImGui::SetItemTooltip("Add Frame");
	ImGui::SameLine();

	if (ImGui::Button("-")) {
		FrameRenderer::removeFrame(canvasManager.getActive());
	}
	ImGui::SetItemTooltip("Remove Frame");
	ImGui::SameLine();
	ImGui::Spacing();
	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_COPY)) {
		FrameRenderer::copyFrame(canvasManager.getActive());
	}
	ImGui::SetItemTooltip("Duplicate Frame");
	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_PLAY)) {
		FrameRenderer::play(canvasManager.getActive());
	}
	ImGui::SetItemTooltip("Play Animation");
	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_CLONE)) {
		FrameRenderer::removeOnionSkin(canvasManager.getActive());
		FrameRenderer::toggleOnionSkin();
		FrameRenderer::updateOnionSkin(canvasManager.getActive());
	}
	ImGui::SetItemTooltip("Toggle Onion Skins");

	ImGui::SameLine();
	ImGui::Text("Frame: %d / %d", FrameRenderer::getCurFrame(), FrameRenderer::getNumFrames());
}

void UI::renderColorSet(CanvasManager& canvasManager, ImVec4* active_color) {
	// color palette section 
		// comment that says Mori Calliope 
	ImGui::Text("Color Set:");
	static ImVec4 palette[32] = {
		// Basics & Brights
		ImVec4(1.00f, 1.00f, 1.00f, 1.00f), ImVec4(1.00f, 0.00f, 0.00f, 1.00f),
		ImVec4(0.00f, 1.00f, 0.00f, 1.00f), ImVec4(0.00f, 0.00f, 1.00f, 1.00f),
		ImVec4(1.00f, 1.00f, 0.00f, 1.00f), ImVec4(1.00f, 0.00f, 1.00f, 1.00f),
		ImVec4(0.00f, 1.00f, 1.00f, 1.00f), ImVec4(0.00f, 0.00f, 0.00f, 1.00f),

		// Deep / Natural Tones
		ImVec4(0.50f, 0.00f, 0.00f, 1.00f), ImVec4(0.00f, 0.50f, 0.00f, 1.00f),
		ImVec4(0.00f, 0.00f, 0.50f, 1.00f), ImVec4(0.50f, 0.50f, 0.00f, 1.00f),
		ImVec4(0.40f, 0.20f, 0.00f, 1.00f), ImVec4(1.00f, 0.50f, 0.00f, 1.00f),
		ImVec4(0.00f, 0.25f, 0.50f, 1.00f), ImVec4(0.20f, 0.20f, 0.20f, 1.00f),

		// Pastels
		ImVec4(1.00f, 0.70f, 0.70f, 1.00f), ImVec4(0.70f, 1.00f, 0.70f, 1.00f),
		ImVec4(0.70f, 0.70f, 1.00f, 1.00f), ImVec4(1.00f, 1.00f, 0.70f, 1.00f),
		ImVec4(1.00f, 0.70f, 1.00f, 1.00f), ImVec4(0.70f, 1.00f, 1.00f, 1.00f),
		ImVec4(1.00f, 0.80f, 0.50f, 1.00f), ImVec4(0.40f, 0.40f, 0.40f, 1.00f),

		// Muted / Grayscale
		ImVec4(0.10f, 0.10f, 0.10f, 1.00f), ImVec4(0.30f, 0.30f, 0.30f, 1.00f),
		ImVec4(0.60f, 0.60f, 0.60f, 1.00f), ImVec4(0.85f, 0.85f, 0.85f, 1.00f),
		ImVec4(0.15f, 0.20f, 0.25f, 1.00f), ImVec4(0.35f, 0.45f, 0.55f, 1.00f),
		ImVec4(0.80f, 0.50f, 0.50f, 1.00f), ImVec4(0.50f, 0.80f, 0.75f, 1.00f)


	};

	// making it wrap with the panel
	float windowMax_x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	float spacing = ImGui::GetStyle().ItemSpacing.x;
	float buttonSize = 20.0f;

	for (int n = 0; n < 32; n++) {
		ImGui::PushID(n);
		if (ImGui::ColorButton("##palette_button", palette[n])) {
			*active_color = palette[n];
		}
		// applying the panel wrapping 
		float lastButton_X = ImGui::GetItemRectMax().x;
		float nextButton_X = lastButton_X + spacing + buttonSize;
		// push to next line if the button does not fit 
		if (n < 31 && nextButton_X < windowMax_x)
			ImGui::SameLine();

		ImGui::PopID();
	}

	color[0] = active_color->x; // R
	color[1] = active_color->y; // G
	color[2] = active_color->z; // B
	color[3] = active_color->w; // A
}

void UI::renderColorWheel(CanvasManager& canvasManager, ImVec4* active_color) {
	// drawing the swatches
	if (ImGui::ColorButton("Primary", primary_color, ImGuiColorEditFlags_None, ImVec2(30, 30))) {
		editing_primary = true;
	}
	// active swatch indicator, orange rectangle
	if (editing_primary) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));

	ImGui::SameLine();

	if (ImGui::ColorButton("Secondary", secondary_color, ImGuiColorEditFlags_None, ImVec2(30, 30))) {
		editing_primary = false;
	}
	if (!editing_primary) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));

	// the actual color picker 
	// using active color so wheel only edits last clicked swatch
	ImGui::ColorPicker4("##wheel", (float*)active_color,
		ImGuiColorEditFlags_PickerHueWheel |
		ImGuiColorEditFlags_NoSidePreview |
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_AlphaBar |
		ImGuiWindowFlags_AlwaysAutoResize);

	color[0] = active_color->x; // R
	color[1] = active_color->y; // G
	color[2] = active_color->z; // B
	color[3] = active_color->w; // A
}

void UI::renderBrushSize(CanvasManager& canvasManager) {
	// * make these wrap with the panel like the color set does * 
	ImGui::SliderInt("Size", &brushSize, 1, 500, "%d", ImGuiSliderFlags_Logarithmic);
	// presets 
	if (ImGui::Button("5"))  brushSize = 5;
	ImGui::SameLine();
	if (ImGui::Button("10")) brushSize = 10;
	ImGui::SameLine();
	if (ImGui::Button("20")) brushSize = 20;

	if (ImGui::Button("30"))  brushSize = 30;
	ImGui::SameLine();
	if (ImGui::Button("40")) brushSize = 40;
	ImGui::SameLine();
	if (ImGui::Button("50")) brushSize = 50;

	if (ImGui::Button("60"))  brushSize = 60;
	ImGui::SameLine();
	if (ImGui::Button("70")) brushSize = 70;
	ImGui::SameLine();
	if (ImGui::Button("80")) brushSize = 80;

	if (ImGui::Button("90"))  brushSize = 90;
	ImGui::SameLine();
	if (ImGui::Button("100")) brushSize = 100;
	ImGui::SameLine();
	if (ImGui::Button("120")) brushSize = 120;

	if (ImGui::Button("150"))  brushSize = 150;
	ImGui::SameLine();
	if (ImGui::Button("200")) brushSize = 200;
	ImGui::SameLine();
	if (ImGui::Button("250")) brushSize = 250;

	if (ImGui::Button("300")) brushSize = 300;
	ImGui::SameLine();
	if (ImGui::Button("350")) brushSize = 350;
	ImGui::SameLine();
	if (ImGui::Button("400")) brushSize = 400;

	if (ImGui::Button("450")) brushSize = 450;
	ImGui::SameLine();
	if (ImGui::Button("500")) brushSize = 500;
}

void UI::renderLayerInfo(CanvasManager& canvasManager) {
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	// save the active canvas for later use
	ImGui::Text("Canvas Size: ");
	ImGui::Text("%dx%d", canvasManager.getActive().getWidth(), canvasManager.getActive().getHeight());

	// Create the layer buttons
	if (ImGui::Button(ICON_FA_LAYER_GROUP "" ICON_FA_PLUS)) {
		// increase the number of layers by 1
		canvasManager.getActive().createLayer();
	}
	ImGui::SetItemTooltip("New Layer");

	// remove a layer button 
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
	if (ImGui::Button(ICON_FA_LAYER_GROUP)) {
		if (canvasManager.getActive().getNumLayers() > 2) {
			// decrease the number of layers by 1
			canvasManager.getActive().removeLayer();
		}
	}

	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Remove Layer");

	for (int i = 1; i < canvasManager.getActive().getNumLayers(); i++) {
		std::string buttonName = ICON_FA_LAYER_GROUP + std::to_string(i);
		if (ImGui::Button(buttonName.c_str())) {
			canvasManager.getActive().selectLayer(i);
		}
	}
	ImGui::Text("Current Layer: %d", canvasManager.getActive().getCurLayer());
}

void UI::renderBrushImports(CanvasManager& canvasManager) {
	// brush import system, will probably get moved when I eventually do a UI overhaul
	if (ImGui::Button(ICON_FA_PAINTBRUSH))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			"ChooseFileDlgKey",
			"Choose File",
			".gbr,.png,.kpp,.jbr"
		);
	}
	ImGui::SetItemTooltip("Import Brush");

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
}

void UI::renderCursorModes(CanvasManager& canvasManager) {
	if (getCursorMode() == CursorMode::Draw) {
		ImGui::Text("State: Draw");
	}

	else if (getCursorMode() == CursorMode::Fill) {
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

	// buttons that change the current cursor mode
	if (ImGui::Button(ICON_FA_PEN)) {
		setCursorMode(CursorMode::Draw);
	}
	ImGui::SetItemTooltip("Pen");

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_FILL_DRIP)) {
		setCursorMode(CursorMode::Fill);
	}
	ImGui::SetItemTooltip("Fill");

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_ERASER)) {
		setCursorMode(CursorMode::Erase);
	}
	ImGui::SetItemTooltip("Erase");

	if (ImGui::Button(ICON_FA_EYE_DROPPER)) {
		setCursorMode(CursorMode::ColorPick);
	}
	ImGui::SetItemTooltip("ColorPick");

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_HAND)) {
		setCursorMode(CursorMode::Pan);
	}
	ImGui::SetItemTooltip("Grab");

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_ARROWS_ROTATE)) {
		setCursorMode(CursorMode::Rotate);
	}
	ImGui::SetItemTooltip("Rotate");

	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_PLUS)) {
		setCursorMode(CursorMode::ZoomIn);
	}
	ImGui::SetItemTooltip("ZoomIn");

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_MINUS)) {
		setCursorMode(CursorMode::ZoomOut);
	}
	ImGui::SetItemTooltip("ZoomOut");
}

std::tuple<bool, float, int> UI::drawDraggableButton(CanvasManager& canvasManager, const char* buttonName, int index){
	float buttonLoc = 400.0f;
	float height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f + 4;
	
	ImGuiID id = ImGui::GetID(buttonName);
    DragState& state = dragStates[id];
	state.index = index;
	if(state.order == 0){
		state.order = state.index;
	}

	int count = dragStates.size();

	// Base position reacts to UI/layout changes
	float baseX = displayWidth - (RightSize - 8);
	float baseY = buttonLoc + (RightSize * .66f) + (index * height);
	float finalY = baseY + state.offsetY;
	

	float boxTop = buttonLoc + (RightSize * .66f);
	float boxBottom = boxTop + (count * height);

	ImVec2 btnPos(baseX, finalY);

	// Draw
	ImGui::SetCursorScreenPos(btnPos);
	bool isPressed = ImGui::Button(buttonName);
	state.notActive = true;
	float newOffset = 0;
	if (ImGui::IsItemActive())
	{
		state.notActive = false;
		float dy = ImGui::GetIO().MouseDelta.y;
		newOffset = state.offsetY + dy;

		if (finalY >= boxTop && finalY <= boxBottom)
		{
			state.offsetY = newOffset;
		}
	}

	// If this button is now outside the valid region, shift it up
	if(state.notActive){
		while ((baseY + state.offsetY) > boxBottom + height)
		{
			state.offsetY -= height;
		}
		while ((baseY + state.offsetY) < boxTop){
			state.offsetY += height;
		} 	
		for (auto& [idA, a] : dragStates){
			if (idA == id || a.notActive == false) continue;
			float aY = (400.0f + (RightSize * .66f)) + (a.index * height) + a.offsetY;
			if (std::abs(aY - finalY) < height * 0.5f){
				state.offsetY -= height;
			}
		}
	}
	if (!state.notActive){
		for (auto& [otherID, other] : dragStates)
		{
			if (otherID == id) continue;
			float otherFinalY = std::min(buttonLoc + (RightSize * .66f) + (other.index * height) + other.offsetY, displayHeight - 29.0f);
			if (std::abs(finalY - otherFinalY) < height * 0.5f)
			{
				if(otherFinalY > finalY){
					other.offsetY = std::round(((other.offsetY / height) * height) - height);
					canvasManager.getActive().swapLayers(other.order, state.order);
					int tempOrder = other.order;
					other.order = state.order;
					state.order = tempOrder;
				}
				else{
					other.offsetY = std::round(((other.offsetY / height) * height) + height);
					canvasManager.getActive().swapLayers(other.order, state.order);
					int tempOrder = other.order;
					other.order = state.order;
					state.order = tempOrder;
				}
			}
		}
	}

	// Snap on release
	if (ImGui::IsItemDeactivated())
	{
		float snapped = std::round(state.offsetY / height) * height;
		state.offsetY = snapped;
	}
	return {isPressed, finalY, index};
}

// ending and cleanup 
void UI::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

// function for closing hotkey
void UI::requestCloseCanvas(int index, CanvasManager& canvasManager)
{
	if (canvasManager.getOpenCanvases()[index].isDirty)
	{
		pendingCloseIndex = index;
		showCloseConfirm = true;
	}
	else
		canvasManager.closeCanvas(index);
}

void UI::requestAppClose(CanvasManager& canvasManager)
{
	pendingAppClose = true;
	for (int i = 0; i < canvasManager.getNumCanvases(); i++) {
		if (canvasManager.getOpenCanvases()[i].isDirty) {
			pendingCloseIndex = i;
			canvasManager.setActiveCanvas(i);
			showCloseConfirm = true;
			return;
		}
	}
	// no dirty canvases, just close immediately
	glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
}