
#include "Window.h"
#include <iostream> 

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
	glViewport(0, 0, w, h);
}

bool Window::create(int w, int h, const char* title) {
	if (!glfwInit()) {
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif 

	m_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
	if (!m_window) {
		return false; 
	}

	glfwMakeContextCurrent(m_window);
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	return true; 

}

void Window::pollEvents() { glfwPollEvents(); }
void Window::swapBuffers() { glfwSwapBuffers(m_window); }
bool Window::shouldClose() const { return glfwWindowShouldClose(m_window); }
GLFWwindow* Window::handle() const { return m_window;}

void Window::destroy() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}