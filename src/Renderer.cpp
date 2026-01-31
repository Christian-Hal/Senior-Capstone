
#include <glad/glad.h> // glad must break include style uniquely 
#include "Renderer.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

#include "Globals.h"
#include "CanvasManager.h"
#include "UI.h"
#include "BrushTool.h"
#include "BrushManager.h"
#include "Zooming.h"


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

// brush manager + brush info
extern BrushManager brushManager;
BrushTool activeBrush;


Camera2d camera;
static bool isPanning = false;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;


static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	// if no renderer    or imgui wants the mouse
	if (!activeRenderer || ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	// if we have a button 				and im gui does NOT want the mouse
	if (button == GLFW_MOUSE_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
	{
		if (action == GLFW_PRESS)
			activeRenderer->isDrawing = true;
		else if (action == GLFW_RELEASE)
		{
			activeRenderer->isDrawing = false;
			hasLastPos = false;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && !ImGui::GetIO().WantCaptureMouse)
	{
		if (action == GLFW_PRESS)
		{
			isPanning = true;
			glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
		}
		else if (action == GLFW_RELEASE)
		{
			isPanning = false;
		}
	}
}

// random mouse setting stuff
int lastDrawnX = 0;
int lastDrawnY = 0;

/*
	Where the main drawing logic currently lies
*/
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	// if no renderer	or it is not drawing 		  or ImGUI wants to use the mouse		or the file is not open
	if (!activeRenderer || ImGui::GetIO().WantCaptureMouse || !activeCanvasManager.hasActive())
		return;


	if (isPanning)
	{
		double dx = xpos - lastMouseX;
		double dy = ypos - lastMouseY;

		camera.offset.x += (float)dx;
		camera.offset.y -= (float)dy;

		lastMouseX = xpos;
		lastMouseY = ypos;
		return;
	}


	if (!activeRenderer->isDrawing) { return; }

	Canvas& curCanvas = activeCanvasManager.getActive();

	float screenX = (float)xpos;
	float screenY = global.get_scr_height() - (float)ypos;


	glm::vec2 canvasCenter(
		curCanvas.getWidth() * 0.5f,
		curCanvas.getHeight() * 0.5f
	);

	glm::vec2 p = { screenX, screenY };

	p -= camera.offset;
	p -= canvasCenter;
	
	float c = cosf(-camera.rotation);
	float s = sinf(-camera.rotation);

	p = {
		p.x * c - p.y * s,
		p.x * s + p.y * c
	};

	p /= camera.zoom;

	p += canvasCenter;


	int x = (int)p.x;
	int y = (int)p.y;


	if (!hasLastPos)
	{
		lastX = x;
		lastY = y;
		hasLastPos = true;
		curCanvas.setPixel(x, y, ui.getColor());
		return;
	}

	int dx = x - lastX;
	int dy = y - lastY;
	int steps = std::max(abs(dx), abs(dy));

	// grab and compute the brush info
	if (brushManager.brushChange == true)
	{
		activeBrush = brushManager.getActiveBrush();
		brushManager.brushChange = false;
	}
	int size = ui.brushSize;
	int w = activeBrush.tipWidth;
	int h = activeBrush.tipHeight;
	int brushSpacing = size * activeBrush.spacing;
	std::vector<float> alpha = activeBrush.tipAlpha;

	int brushCenter_x = w / 2;
	int brushCenter_y = h / 2;

	// for each step between the last position and current position
	for (int i = 0; i <= steps; i++)
	{
		int baseX = lastX + dx * i / steps - brushCenter_x * size;
		int baseY = lastY + dy * i / steps - brushCenter_y * size;

		float distance = sqrt(((lastDrawnX - baseX) * (lastDrawnX - baseX)) +  ((lastDrawnY - baseY) * (lastDrawnY - baseY)));
		if (distance < brushSpacing)
			continue;

		// for each row in the brush mask
		for (int r = 0; r < h; r++)
		{
			// for each column in the brush mask
			for (int c = 0; c < w; c++)
			{
				// if the current index is part of the pattern
				float a = alpha[r * w + c];
				if (a > 0.01f) 
				{
					for (int sy = 0; sy < size; sy++)
					{
						for (int sx = 0; sx < size; sx++)
						{
							// calculate the pixel x and y on the canvas
							int px = baseX + c * size + sx;
                    		int py = baseY + r * size + sy;
							
                    		curCanvas.setPixel(px, py, ui.getColor());
						}
					}

					lastDrawnX = baseX;
					lastDrawnY = baseY;
				}
			}
		}
	}

	lastX = x;
	lastY = y;
}


static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	// Rotate when holding R
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		camera.rotation += (float)yoffset * 0.05f;
		return;
	}

	const float zoomSpeed = 0.1f;
	float oldZoom = camera.zoom;

	camera.zoom *= (1.0f + (float)yoffset * zoomSpeed);
	camera.zoom = std::clamp(camera.zoom, 0.1f, 10.0f);

	// Mouse position (screen space)
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);

	glm::vec2 mouseScreen(
		(float)mx,
		(float)(global.get_scr_height() - my)
	);

	Canvas& canvas = activeCanvasManager.getActive();

	glm::vec2 canvasCenter(
		canvas.getWidth() * 0.5f,
		canvas.getHeight() * 0.5f
	);

	// --- Build OLD view matrix ---
	glm::mat4 oldView(1.0f);
	oldView = glm::translate(oldView, glm::vec3(camera.offset, 0.0f));
	oldView = glm::translate(oldView, glm::vec3(canvasCenter, 0.0f));
	oldView = glm::rotate(oldView, camera.rotation, glm::vec3(0, 0, 1));
	oldView = glm::scale(oldView, glm::vec3(oldZoom, oldZoom, 1.0f));
	oldView = glm::translate(oldView, glm::vec3(-canvasCenter, 0.0f));

	// Convert mouse to world/canvas space
	glm::vec4 world =
		glm::inverse(oldView) * glm::vec4(mouseScreen, 0.0f, 1.0f);

	// --- Build NEW view matrix ---
	glm::mat4 newView(1.0f);
	newView = glm::translate(newView, glm::vec3(camera.offset, 0.0f));
	newView = glm::translate(newView, glm::vec3(canvasCenter, 0.0f));
	newView = glm::rotate(newView, camera.rotation, glm::vec3(0, 0, 1));
	newView = glm::scale(newView, glm::vec3(camera.zoom, camera.zoom, 1.0f));
	newView = glm::translate(newView, glm::vec3(-canvasCenter, 0.0f));

	// Where that world point ends up after zoom
	glm::vec4 newScreen = newView * world;

	// Offset correction so mouse stays fixed
	glm::vec2 delta =
		mouseScreen - glm::vec2(newScreen);

	camera.offset += delta;
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
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, scrollCallBack);

	return true;
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

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

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

	view = glm::translate(view, glm::vec3(camera.offset, 0.0f));
	view = glm::translate(view, glm::vec3(canvasCenter, 0.0f));
	view = glm::rotate(view, camera.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::scale(view, glm::vec3(camera.zoom, camera.zoom, 1.0f));
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