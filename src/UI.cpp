
#include "UI.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// state 

enum Mode {
	DRAW, 
	ERASE
};

static Mode drawState = DRAW;  

// UI 

void UI::init(GLFWwindow* window) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark(); 

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410 core");
}

void UI::draw() {

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown(); 
	ImGui::DestroyContext();
}