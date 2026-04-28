
#include <glad/glad.h> // glad must break include style uniquely 
#include "Renderer.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <list>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "stb_image.h"


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

// static const char* fragmentShaderSource = R"(
// #version 330 core
// in vec2 TexCoord;
// out vec4 FragColor;
// uniform sampler2D canvasTex;

// void main(){
//     FragColor = texture(canvasTex, TexCoord);
// }
// )";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D canvasTex;
uniform sampler2D paperTex;

void main(){
    vec4 canvasColor = texture(canvasTex, TexCoord);
    vec4 paperColor = texture(paperTex, TexCoord);

	if (canvasColor.a > 0.001) 
        canvasColor.rgb /= canvasColor.a;
    
    // Blend using canvas's alpha channel - canvas appears on top
    FragColor = mix(paperColor, canvasColor, canvasColor.a);
}
)";


// global instance reference
extern Globals global;

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
	glGenTextures(1, &canvasTexture);

	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &paperTexture);

	glBindTexture(GL_TEXTURE_2D, paperTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}


void Renderer::beginFrame(CanvasManager& canvasManager)
{
	// clears the screen
	glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// render the active canvas
	if (canvasManager.hasActive())
	{
		// if the active canvas' paper has changed then update the texture
		if (canvasManager.paperChange) {
			uploadExtraTexture(canvasManager.getActive());
			canvasManager.paperChange = false;
		}

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



// uploads the canvas pixel data and paper layer to the quad texture
void Renderer::uploadCanvasTexture(const Canvas& canvas) {

	// set the active texture and bind the canvas data to the texture
	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, canvas.getWidth(), canvas.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.getData());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Renderer::uploadExtraTexture(const Canvas& canvas) {
	// upload the anim tempalte if its needed
	if (canvas.isUsingAnimTemplate()) {
		// load and generate the texture
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load("assets/Animation_Template_PNG.png", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	} else { // other wise just use the paper color
		// set the active texture and bind the paper color to the texture
		glBindTexture(GL_TEXTURE_2D, paperTexture);
		
		// Create a vector filled with the canvas's paper (background) color
		Color paperColor = canvas.getBackgroundColor();
		std::vector<Color> paperLayerData(canvas.getWidth() * canvas.getHeight(), paperColor);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, canvas.getWidth(), canvas.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, paperLayerData.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

// uploads the canvas pixel data and paper layer to the quad texture
void Renderer::uploadAnimTextures(const Canvas& canvas) {

	// set the active texture and bind the canvas data to the texture
	glBindTexture(GL_TEXTURE_2D, canvasTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, canvas.getWidth(), canvas.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.getData());

	// load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/Animation_Template_PNG.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}


// renders the quad using the uploaded pixel data
void Renderer::renderCanvas(const Canvas& canvas)
{
	uploadCanvasTexture(canvas);

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

	// check to see how the mag filter should be handled based on the zoom level
	if (canvas.zoom < 0.5f) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "canvasTex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paperTexture);
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "paperTex"), 1);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}