
#include <glad/glad.h> // glad must break include style uniquely 
#include "Renderer.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <list>

#include "Globals.h"
#include "CanvasManager.h"
#include "UI.h"
#include "Zooming.h"
#include "DrawEngine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


// shader sources 
static const char* vertexShaderSource = R"(
#version 330 core
layout(location=0) in vec2 pos;
layout(location=1) in vec2 uv;

uniform mat4 u_MVP;

out vec2 TexCoord;

void main(){
    TexCoord = uv;
    gl_Position = u_MVP * vec4(pos,0.0,1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D canvasTex;

void main(){
    FragColor = texture(canvasTex, TexCoord);
}
)";


// global instance reference
extern Globals global;
extern DrawEngine drawEngine;

// Framebuffer Settings
int fbWidth = 0, fbHeight = 0;

GLuint lineVAO, lineVBO;
unsigned int oldShaderProgram = 0;

static Renderer* activeRenderer = nullptr;
static CanvasManager activeCanvasManager;
static UI ui;

static bool hasLastPos = false;
static int lastX = 0;
static int lastY = 0;

//Camera2d camera;

// callback for mouse button reading
static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	Canvas& curCanvas = activeCanvasManager.getActive();
	// if no renderer    or imgui wants the mouse
	if (!activeRenderer || ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	UI::CursorMode mode = ui.getCursorMode();

	// if we have a button 				and im gui does NOT want the mouse
	if (button == GLFW_MOUSE_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
	{
		if (action == GLFW_PRESS)
		{
			// for drawing and erasing
			if ((mode == UI::CursorMode::Draw || mode == UI::CursorMode::Erase) && activeCanvasManager.hasActive())
			{
				activeRenderer->isDrawing = true;
				drawEngine.start();
			}
			
			// for pan and zoom and come extra logic for smooth rotation
			else if (mode == UI::CursorMode::Pan || mode == UI::CursorMode::Rotate)
			{
				isPanning = true;
				glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

				if (mode == UI::CursorMode::Rotate)
				{

					glm::vec2 canvasCenter(
						curCanvas.getWidth() * 0.5f + curCanvas.offset.x,
						curCanvas.getHeight() * 0.5f + curCanvas.offset.y
					);

					float mx = (float)lastMouseX;
					float my = global.get_scr_height() - (float)lastMouseY;

					lastAngle = atan2f(my - canvasCenter.y, mx - canvasCenter.x);
				}
			}	

			// for click zoom in and out plus logic for it
			else if (mode == UI::CursorMode::ZoomIn || mode == UI::CursorMode::ZoomOut)
			{
				const float zoomStep = 1.2f;
				float oldZoom = curCanvas.zoom;

				if (mode == UI::CursorMode::ZoomIn)
					curCanvas.zoom *= zoomStep;
				else
					curCanvas.zoom /= zoomStep;

				curCanvas.zoom = std::clamp(curCanvas.zoom, 0.1f, 10.0f);

				double mx, my;
				glfwGetCursorPos(window, &mx, &my);

				glm::vec2 mouseScreen(
					(float)mx,
					(float)(global.get_scr_height() - my)
				);

				glm::vec2 canvasCenter(
					curCanvas.getWidth() * 0.5f,
					curCanvas.getHeight() * 0.5f
				);

				glm::mat4 oldView(1.0f);
				oldView = glm::translate(oldView, glm::vec3(curCanvas.offset, 0.0f));
				oldView = glm::translate(oldView, glm::vec3(canvasCenter, 0.0f));
				oldView = glm::rotate(oldView, curCanvas.rotation, glm::vec3(0, 0, 1));
				oldView = glm::scale(oldView, glm::vec3(oldZoom));
				oldView = glm::translate(oldView, glm::vec3(-canvasCenter, 0.0f));

				glm::vec4 world =
					glm::inverse(oldView) * glm::vec4(mouseScreen, 0.0f, 1.0f);

				glm::mat4 newView(1.0f);
				newView = glm::translate(newView, glm::vec3(curCanvas.offset, 0.0f));
				newView = glm::translate(newView, glm::vec3(canvasCenter, 0.0f));
				newView = glm::rotate(newView, curCanvas.rotation, glm::vec3(0, 0, 1));
				newView = glm::scale(newView, glm::vec3(curCanvas.zoom));
				newView = glm::translate(newView, glm::vec3(-canvasCenter, 0.0f));

				glm::vec4 newScreen = newView * world;

				curCanvas.offset += mouseScreen - glm::vec2(newScreen);
				return;
			}
		}
		else if (action == GLFW_RELEASE && activeCanvasManager.hasActive())
		{
			activeRenderer->isDrawing = false;
			drawEngine.stop();
			isPanning = false;
		}
	}
}

/*
	Where the main drawing logic currently lies 
*/
static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
	// if no renderer	  or ImGUI wants to use the mouse	 or the file is not open			or it is not drawing
	if (!activeRenderer || ImGui::GetIO().WantCaptureMouse || !activeCanvasManager.hasActive())// || !drawEngine.isDrawing())
		return;

	UI::CursorMode mode = ui.getCursorMode();
	Canvas& curCanvas = activeCanvasManager.getActive();

	//if (drawEngine.isDrawing()) drawEngine.doStamp = false;
	
	// panning and rotation logic cursor logic
	if (isPanning)
	{
		double dx = xpos - lastMouseX;
		double dy = ypos - lastMouseY;

		if (mode == UI::CursorMode::Pan)
		{
			curCanvas.offset.x += (float)dx;
			curCanvas.offset.y -= (float)dy;
		}

		else if (mode == UI::CursorMode::Rotate)
		{

			glm::vec2 canvasCenter(
				curCanvas.getWidth() * 0.5f + curCanvas.offset.x,
				curCanvas.getHeight() * 0.5f + curCanvas.offset.y
			);

			float mx = (float)xpos;
			float my = global.get_scr_height() - (float)ypos;

			float angle = atan2f(my - canvasCenter.y, mx - canvasCenter.x);
			float delta = angle - lastAngle;

			curCanvas.rotation += delta;
			lastAngle = angle;
		}

		lastMouseX = xpos;
		lastMouseY = ypos;
		return;
	}

	// for tempoary zoom logic
	if (isZoomDragging)
	{
		double dy = ypos - lastZoomMouseY;
		lastZoomMouseY = ypos;

		const float zoomSpeed = 0.005f;
		float oldZoom = curCanvas.zoom;

		curCanvas.zoom *= (1.0f - (float)dy * zoomSpeed);
		curCanvas.zoom = std::clamp(curCanvas.zoom, 0.1f, 10.0f);

		// Zoom around mouse position
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);

		glm::vec2 mouseScreen(
			(float)mx,
			(float)(global.get_scr_height() - my)
		);

		glm::vec2 canvasCenter(
			curCanvas.getWidth() * 0.5f,
			curCanvas.getHeight() * 0.5f
		);

		glm::mat4 oldView(1.0f);
		oldView = glm::translate(oldView, glm::vec3(curCanvas.offset, 0.0f));
		oldView = glm::translate(oldView, glm::vec3(canvasCenter, 0.0f));
		oldView = glm::rotate(oldView, curCanvas.rotation, glm::vec3(0, 0, 1));
		oldView = glm::scale(oldView, glm::vec3(oldZoom));
		oldView = glm::translate(oldView, glm::vec3(-canvasCenter, 0.0f));

		glm::vec4 world =
			glm::inverse(oldView) * glm::vec4(mouseScreen, 0.0f, 1.0f);

		glm::mat4 newView(1.0f);
		newView = glm::translate(newView, glm::vec3(curCanvas.offset, 0.0f));
		newView = glm::translate(newView, glm::vec3(canvasCenter, 0.0f));
		newView = glm::rotate(newView, curCanvas.rotation, glm::vec3(0, 0, 1));
		newView = glm::scale(newView, glm::vec3(curCanvas.zoom));
		newView = glm::translate(newView, glm::vec3(-canvasCenter, 0.0f));

		glm::vec4 newScreen = newView * world;
		curCanvas.offset += mouseScreen - glm::vec2(newScreen);

		return;
	}
}

static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	Canvas& curCanvas = activeCanvasManager.getActive();
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	// Rotate when holding R
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		curCanvas.rotation += (float)yoffset * 0.05f;
		return;
	}

	const float zoomSpeed = 0.1f;
	float oldZoom = curCanvas.zoom;

	curCanvas.zoom *= (1.0f + (float)yoffset * zoomSpeed);
	curCanvas.zoom = std::clamp(curCanvas.zoom, 0.1f, 10.0f);

	// Mouse position (screen space)
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);

	glm::vec2 mouseScreen(
		(float)mx,
		(float)(global.get_scr_height() - my)
	);


	glm::vec2 canvasCenter(
		curCanvas.getWidth() * 0.5f,
		curCanvas.getHeight() * 0.5f
	);

	// --- Build OLD view matrix ---
	glm::mat4 oldView(1.0f);
	oldView = glm::translate(oldView, glm::vec3(curCanvas.offset, 0.0f));
	oldView = glm::translate(oldView, glm::vec3(canvasCenter, 0.0f));
	oldView = glm::rotate(oldView, curCanvas.rotation, glm::vec3(0, 0, 1));
	oldView = glm::scale(oldView, glm::vec3(oldZoom, oldZoom, 1.0f));
	oldView = glm::translate(oldView, glm::vec3(-canvasCenter, 0.0f));

	// Convert mouse to world/canvas space
	glm::vec4 world =
		glm::inverse(oldView) * glm::vec4(mouseScreen, 0.0f, 1.0f);

	// --- Build NEW view matrix ---
	glm::mat4 newView(1.0f);
	newView = glm::translate(newView, glm::vec3(curCanvas.offset, 0.0f));
	newView = glm::translate(newView, glm::vec3(canvasCenter, 0.0f));
	newView = glm::rotate(newView, curCanvas.rotation, glm::vec3(0, 0, 1));
	newView = glm::scale(newView, glm::vec3(curCanvas.zoom, curCanvas.zoom, 1.0f));
	newView = glm::translate(newView, glm::vec3(-canvasCenter, 0.0f));

	
	glm::vec4 newScreen = newView * world;
	curCanvas.offset += mouseScreen - glm::vec2(newScreen);
}

//keyboard callbacks to set up hotkeys for switching cursorModes and temporary shortcut for zoom
static void keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Rotate);
		return;
	}

	else if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Pan);
		return;
	}

	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Draw);
		return;
	}

	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		ui.setCursorMode(UI::CursorMode::Erase);
		return;
	}

	// UNDO BUTTON!!!!!!! checking for control + z
	else if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL) && action == GLFW_PRESS)
	{
		activeCanvasManager.undo();
		return;
	}
	// REDO BUTTON!!!!!!! checking for control + y
	else if (key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL) && action == GLFW_PRESS)
	{
		activeCanvasManager.redo();
		return;
	}

	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS &&
			(mods & GLFW_MOD_CONTROL))
		{
			isZoomDragging = true;
			glfwGetCursorPos(window, nullptr, &lastZoomMouseY);
		}

		if (action == GLFW_RELEASE)
		{
			isZoomDragging = false;
		}
	}

	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
	{
		if (action == GLFW_RELEASE)
			isZoomDragging = false;
	}
}

// compile the vertex and fragment shaders 
static unsigned int compileShader(unsigned int type, const char* source) {

	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
	}
	return shader;
}



// 
bool Renderer::init(GLFWwindow* window, Globals& g_inst)
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		return false;
	}

	// ----- Shaders -----
	// compiles the shaders
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	// creates the shader program and attatches the shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// removes the unneeded shader data
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// ----- Texture Setup -----
	glGenTextures(0, &canvasTexture);
	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	activeRenderer = this;
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetKeyCallback(window, keyboardCallBack);

	return true;
}



static void centerCamera(Canvas& canvas)
{

	canvas.zoom = std::min((float)global.get_scr_width() / canvas.getWidth(), (float)global.get_scr_height() / canvas.getHeight()) * 0.95f;
	 
	canvas.rotation = 0.0f;

	glm::vec2 screenCenter(
		global.get_scr_width() * 0.5f,
		global.get_scr_height() * 0.5f
	);

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f,
		canvas.getHeight() * 0.5f
	);

	canvas.offset = screenCenter - canvasCenter;
}



//
void Renderer::beginFrame(CanvasManager& canvasManager)
{
	activeCanvasManager = canvasManager;

	// clears the screen
	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	// render the active canvas
	if (canvasManager.hasActive())
	{
		// if the active canvas has chaneged then recreate the vbo/vao
		if (canvasManager.canvasChange || global.dirtyScreen) {
			createCanvasQuad(canvasManager.getActive());
			//centerCamera(activeCanvasManager.getActive());
			canvasManager.canvasChange = false;
			global.dirtyScreen = false;
		}

		renderCanvas(canvasManager.getActive());
	}
}



void Renderer::endFrame() {
	glBindVertexArray(0);
}



void Renderer::shutdown() {
	// delete the data no longer needed
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shaderProgram);
}



/*
	Canvas Rendering functions.
	
	Creates the VAO and VBO for the canvas quad.
*/
void Renderer::createCanvasQuad(const Canvas& canvas)
{
	float w = (float)canvas.getWidth();
	float h = (float)canvas.getHeight();

	float quadVerts[] = {
		// pos (pixels)     // uv
		0.f, 0.f,          0.f, 0.f,
		w,   0.f,          1.f, 0.f,
		w,   h,            1.f, 1.f,

		0.f, 0.f,          0.f, 0.f,
		w,   h,            1.f, 1.f,
		0.f, h,            0.f, 1.f
	};



	// generate and bind the vbo and vao
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



// uploads the canvas pixel data to the quad texture
void Renderer::uploadTexture(const Canvas& canvas) {

	// sets the active texture
	glBindTexture(GL_TEXTURE_2D, canvasTexture);

	// binds the image data to the texture
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA8, canvas.getWidth(), canvas.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.getData());

}



// renders the quad using the uploaded pixel data
void Renderer::renderCanvas(const Canvas& canvas)
{
	////// if no active canvas then don't do any of this
	////// will give the effect of a "main screen" when no file is open

	// uplaod the canvas to the texture
	uploadTexture(canvas);

	glm::mat4 projection = glm::ortho(
		0.0f,
		(float)global.get_scr_width(),
		0.0f,
		(float)global.get_scr_height()
	);

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f,
		canvas.getHeight() * 0.5f
	);

	glm::mat4 view = glm::mat4(1.0f);

	view = glm::translate(view, glm::vec3(canvas.offset, 0.0f));
	view = glm::translate(view, glm::vec3(canvasCenter, 0.0f));
	view = glm::rotate(view, canvas.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::scale(view, glm::vec3(canvas.zoom, canvas.zoom, 1.0f));
	view = glm::translate(view, glm::vec3(-canvasCenter, 0.0f));

	glm::mat4 mvp = projection * view;

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(
		glGetUniformLocation(shaderProgram, "u_MVP"),
		1,
		GL_FALSE,
		glm::value_ptr(mvp)
	);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "canvasTex"), 0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}