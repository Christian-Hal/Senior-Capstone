
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "Globals.h"
#include <iostream>
#include <vector>

// shader sources 
static const char* vertexShaderSource = R"(
#version 330 core 
layout (location = 0) in vec3 aPos;

void main(){
	gl_Position = vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor; 

void main(){
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);

}
)";


static Globals global;

// Framebuffer Settings
int fbWidth = 0, fbHeight = 0;

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

bool Renderer::init(GLFWwindow* window, Globals g_inst) 
{
	global = g_inst;
	fbWidth = global.get_canvas_x();
	fbHeight = global.get_canvas_y();

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		return false; 
	}

	// canvas geometry
	float w = (float)fbWidth;
	float h = (float)fbHeight; 

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

    // generate and bind the vbo and vao
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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

	// --- Framebuffer Setup ---
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

/* void Renderer::createFramebuffer() {


} */

unsigned int Renderer::beginFrame() {
    // clears the screen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    // activates shader program and draws the traingle
	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
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

	return colorTexture;
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
