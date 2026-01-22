
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

#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


// shader sources 
static const char* vertexShaderSource = R"(
#version 330 core
layout(location=0) in vec2 pos;
layout(location=1) in vec2 uv;

out vec2 TexCoord;

void main(){
    TexCoord = uv;
    gl_Position = vec4(pos,0,1);
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

static BrushTool activeBrush = BrushTool(5, 5);


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

	
}



static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	// if no renderer	or it is not drawing 		  or ImGUI wants to use the mouse		or the file is not open
	if (!activeRenderer || !activeRenderer->isDrawing || ImGui::GetIO().WantCaptureMouse || !activeCanvasManager.hasActive())
		return;


	Canvas& curCanvas = activeCanvasManager.getActive();
	
	float centerX = global.get_scr_width() * 0.5f;
	float centerY = global.get_scr_height() * 0.5f;
	float cW = curCanvas.getWidth() * 0.5;
	float cH = curCanvas.getHeight() * 0.5;

	float canvasL = centerX - cW;
	float canvasR = centerY - cH;

	float relX = xpos - canvasL;
	float relY = ypos - canvasR;

	//if (relX < 0 || relX >= curCanvas.getWidth() || relY < 0 || relY >= curCanvas.getHeight())
	//	return;
	
	//std::cout << "x,y" << xpos << ", " << ypos << std::endl;

	int x = static_cast<int>(relX);
	int y = static_cast<int>(curCanvas.getHeight() - 1 - static_cast<int>(relY));
	
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
	int size = ui.brushSize;
	int w = activeBrush.width;
	int h = activeBrush.height;
	std::vector<int> mask = activeBrush.mask;

	int brushCenter_x = w / 2;
	int brushCenter_y = h / 2;

	for (int i = 0; i <= steps; i++)
	{
		// for reach row in the brush mask
		for (int r = 0; r < h; r++)
		{
			// for each column in the brush mask
			for (int c = 0; c < w; c++)
			{
				// if the current index is part of the pattern
				if (mask[r * w + c] == 1) 
				{
					for (int sy = 0; sy < size; sy++)
					{
						for (int sx = 0; sx < size; sx++)
						{
							// calculate the pixel x and y on the canvas
							int px = (lastX + dx * i / steps) + (c - brushCenter_x) + sx;
                        	int py = (lastY + dy * i / steps) + (r - brushCenter_y) + sy;
							curCanvas.setPixel(px, py, ui.getColor());
						}
					}
				}
			}
		}
	}
	lastX = x;
	lastY = y;
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
	//global = g_inst;

	// fb size equal to user input x and y 
	fbWidth = global.get_canvas_x();
	fbHeight = global.get_canvas_y();

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


	// --- Framebuffer Setup ---
	Renderer::createFramebuffer(fbWidth, fbHeight);


	activeRenderer = this;
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallback);



	return true;
}



// creates the center framebuffer 
bool Renderer::createFramebuffer(float fbWidth, float fbHeight) {


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbWidth, fbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		colorTexture,
		0
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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





///// Canvas Rendering functions
// creates the VAO and VBO for the canvas quad
void Renderer::createCanvasQuad(const Canvas& canvas)
{
	// get screen center
	float centerX = global.get_scr_width() * 0.5f;
	float centerY = global.get_scr_height() * 0.5f;

	// canvas geometry
	float cW = canvas.getWidth() * 0.5f;
	float cH = canvas.getHeight() * 0.5f;

	float screenW = global.get_scr_width();
	float screenH = global.get_scr_height();

	float quadVerts[] = {
		// position coords																	// texture coords
		// this is a formula that takes the Pixel coordinates and converts them to NDC
		(centerX - cW) / (screenW * 0.5f) - 1.f, (centerY - cH) / (screenH * 0.5f) - 1.f, 	0.f, 0.f,
		(centerX + cW) / (screenW * 0.5f) - 1.f, (centerY - cH) / (screenH * 0.5f) - 1.f, 	1.f, 0.f,
		(centerX + cW) / (screenW * 0.5f) - 1.f, (centerY + cH) / (screenH * 0.5f) - 1.f, 	1.f, 1.f,

		(centerX - cW) / (screenW * 0.5f) - 1.f, (centerY - cH) / (screenH * 0.5f) - 1.f, 	0.f, 0.f,
		(centerX + cW) / (screenW * 0.5f) - 1.f, (centerY + cH) / (screenH * 0.5f) - 1.f, 	1.f, 1.f,
		(centerX - cW) / (screenW * 0.5f) - 1.f, (centerY + cH) / (screenH * 0.5f) - 1.f, 	0.f, 1.f
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

	// activate the texture and then send it to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "canvasTex"), 0);

	// render the quad
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}