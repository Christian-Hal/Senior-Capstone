#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>

// Vertex Shader source
const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

// Fragment Shader source
const char* fragmentShaderSource = R"(
#version 410 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// global variables
enum Mode { DRAW, ERASE };
Mode drawState = DRAW;

int main()
{
    // initialize glad
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // create the actual window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();  // or ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core"); // same as your shader version


    // vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // left
         0.5f, -0.5f, 0.0f,  // right
         0.0f,  0.5f, 0.0f   // top
    };

    // create vao and vbo and bind the buffer data
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // create the shader program
    unsigned int myShader = glCreateProgram();
    glAttachShader(myShader, vertexShader);
    glAttachShader(myShader, fragmentShader);
    glLinkProgram(myShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // RENDER LOOP
    while (!glfwWindowShouldClose(window))
{
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Your ImGui UI
    ImGui::Begin("Side Panel");
    switch (drawState)
    {
        case DRAW:
            ImGui::Text("Current State: Draw");
            break;
        case ERASE:
            ImGui::Text("Current State: Erase");
            break;
    }
    if (ImGui::Button("Draw"))
    {
        drawState = DRAW;
    }

    if (ImGui::Button("Erase"))
    {
        drawState = ERASE;
    }
    ImGui::End();
	


    // OpenGL rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(myShader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}


    // clean up step
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(myShader);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
