


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include <iostream>

// shader sources 
static const char* vertexShaderSource = R"(
#version 410 core 
layout (location = 0) in vec3 aPos;

void main(){
	gl_Position = vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 410 core 
out vec4 FragColor; 

void main(){
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);

}
)";

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

bool Renderer::init(GLFWwindow* window, int W, int H) 
{

	fbWidth = W;
	fbHeight = H;

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		return false; 
	}

	// geometry 
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

    // generate and bind the vbo and vao
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
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
	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	glLinkProgram(m_shaderProgram);

    // removes the unneeded shader data
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// --- Framebuffer Setup ---
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorTexture,
        0
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

unsigned int Renderer::beginFrame() {
    /*/ clears the screen
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    // activates shader program and draws the traingle
	glUseProgram(m_shaderProgram);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3); */

	// render the triangle into the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, fbWidth, fbHeight);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shaderProgram);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return m_colorTexture;
}

void Renderer::endFrame() {
	glBindVertexArray(0);
}

void Renderer::shutdown() {
    // delete the data no longer needed
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteProgram(m_shaderProgram);
}
