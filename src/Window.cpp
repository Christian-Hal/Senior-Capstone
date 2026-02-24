
#include "Window.h"

#include <iostream> 

#include "Globals.h"

// global instance reference
extern Globals global;



// framebuffer size callback
static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
	glViewport(0, 0, w, h);
}

// window size callback
static void window_size_callback(GLFWwindow*, int w, int h) {
	global.set_scr_width(w);
	global.set_scr_height(h);

	global.dirtyScreen = true;
}


bool Window::create(int w, int h, const char* title, Globals gb) {
	if (!glfwInit()) {
		return false;
	}

	//global = gb;

	// establishing openGL version 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	// adds HiDPI support
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif 

	// specifying and creating window 
	m_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
	if (!m_window) {
		return false; 
	}

	glfwMakeContextCurrent(m_window);
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(m_window, window_size_callback);
	
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