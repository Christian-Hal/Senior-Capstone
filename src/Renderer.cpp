
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Globals.h"
#include "CanvasManager.h"
#include <iostream>
#include <vector>

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

// OLD SHADER CODE
static const char* oldVertexShaderSource = R"(
#version 330 core 
layout (location = 0) in vec3 aPos;

void main(){
	gl_Position = vec4(aPos, 1.0);
}
)";

static const char* oldFragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor; 

void main(){
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

// making an instances of classes
static Globals global;

// Framebuffer Settings
int fbWidth = 0, fbHeight = 0;




GLuint lineVAO, lineVBO;
unsigned int oldShaderProgram = 0;

static Renderer* activeRenderer = nullptr;

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
		activeRenderer->isDrawing = (action == GLFW_PRESS);
	}
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	// if no renderer	or it is not drawing 		  or ImGUI wants to use the mouse
	if (!activeRenderer || !activeRenderer->isDrawing || ImGui::GetIO().WantCaptureMouse)
		return;

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	// Convert screen → NDC
	float x = (xpos / w) * 2.0f - 1.0f;
	float y = 1.0f - (ypos / h) * 2.0f;
	
	activeRenderer->drawVertices.push_back(x);
	activeRenderer->drawVertices.push_back(y);
	activeRenderer->drawVertices.push_back(0.0f);
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
	global = g_inst;

	// fb size equal to user input x and y 
	fbWidth = global.get_canvas_x();
	fbHeight = global.get_canvas_y();

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		return false; 
	}

	// canvas geometry
	float w = (float)fbWidth;
	float h = (float)fbHeight; 

	float quadVerts[] = {
		// position	 // tex coords
		-1.f, -1.f,   0.f, 0.f,
		1.f, -1.f,   1.f, 0.f,
		1.f,  1.f,   1.f, 1.f,

		-1.f, -1.f,   0.f, 0.f,
		1.f,  1.f,   1.f, 1.f,
		-1.f,  1.f,   0.f, 1.f
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

	// ----- Old Shaders and VAO/VBO for gunters drawing code -----
    // compiles the shaders
	unsigned int oldVertexShader = compileShader(GL_VERTEX_SHADER, oldVertexShaderSource);
	unsigned int oldFragmentShader = compileShader(GL_FRAGMENT_SHADER, oldFragmentShaderSource);

    // creates the shader program and attatches the shaders
	oldShaderProgram = glCreateProgram();
	glAttachShader(oldShaderProgram, oldVertexShader);
	glAttachShader(oldShaderProgram, oldFragmentShader);
	glLinkProgram(oldShaderProgram);

    // removes the unneeded shader data
	glDeleteShader(oldVertexShader);
	glDeleteShader(oldFragmentShader);

	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0); // x,y,z
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


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
void Renderer::beginFrame(CanvasManager& canvasManager) {

    // clears the screen
	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

    // activates shader program and draws the traingle
	// glUseProgram(shaderProgram);
	// glBindVertexArray(vao);

	renderCanvas(canvasManager.getActive());
	
	if (!drawVertices.empty())
	{
		glUseProgram(oldShaderProgram);
		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(
			GL_ARRAY_BUFFER,
			drawVertices.size() * sizeof(float),
			drawVertices.data(),
			GL_DYNAMIC_DRAW
		);

		glDrawArrays(GL_LINE_STRIP, 0, drawVertices.size() / 3);
	}

	//glDrawArrays(GL_TRIANGLES, 0, 3);

	/*/ render the triangle into the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbWidth, fbHeight);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); */
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

void Renderer::getFrameData()
{
	// checks if the buffer size is not 0
	// when the app first runs these two are initialized to zero as a sort of "file is not open"
	// so this is an easy fix until we get the state system fully set up and can know when a file is or isnt open
	if (fbWidth == 0 || fbHeight == 0)
        return;
	
	// bind to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	// create a vector that can save all the pixel info
    std::vector<unsigned char> pixels(fbWidth * fbHeight * 4); // length * width * RGBA

	// read the pixels on the framebuffer and save them to vector
    glReadPixels(0, 0, fbWidth, fbHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	// unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	std::cout << "number of pixels: "<< pixels.size() << std::endl;

    // Example: print first pixel
    std::cout << "First pixel RGBA: "
              << (int)pixels[0] << ", "
              << (int)pixels[1] << ", "
              << (int)pixels[2] << ", "
              << (int)pixels[3] << std::endl;
}

// Canvas Rendering functions
void Renderer::uploadTexture(const Canvas& canvas) {
	
	// sets the active texture
	glBindTexture(GL_TEXTURE_2D, canvasTexture);

	// binds the image data to the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas.getWidth(), canvas.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.getData());

}

void Renderer::renderCanvas(const Canvas& canvas)
{
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