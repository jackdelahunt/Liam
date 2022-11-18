#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

Window make_window(
	s32 width, s32 height, 
	const char* title, 
	void (*key_callback)(GLFWwindow*, s32, s32, s32, s32),
	void (*mouse_button_callback)(GLFWwindow*, s32, s32, s32),
	void (*cursor_position_callback)(GLFWwindow*, f64, f64),
	void (*window_size_callback)(GLFWwindow*, s32, s32)) {
    
    if (!glfwInit()) {
		LOG_ERR_ENGINE("%s", "Could not init glfw.. crashing");
		exit(EXIT_FAILURE);
	}

	GLFWwindow* glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!glfw_window) {
		glfwTerminate();
		LOG_ERR_ENGINE("%s", "Could not create glfw window.. crashing");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfw_window);
	s32 status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	ASSERT_ENGINE(status, "%s", "Glad could not load glfw proc address");

    LOG_INFO_ENGINE("{} :: {} :: {}", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

	glClearColor(0.3, 0.3, 0.3, 0);
	glfwSwapInterval(1); // vsync, set to 0 to turn off

    Window window = Window {
        .glfw_window = glfw_window,
        .width = width,
        .height = height,
        .title = title
    };

	// callbacks
	// callbacks from glfw to handle changes from the window
	glfwSetKeyCallback(window.glfw_window, key_callback);
	glfwSetMouseButtonCallback(window.glfw_window, mouse_button_callback);
	glfwSetCursorPosCallback(window.glfw_window, cursor_position_callback);
	glfwSetWindowSizeCallback(window.glfw_window, window_size_callback);

	// setup imgui, this is just some boiler plate that needs
	// to be done, make use of the docking and congifures it
	// for glfw and open gl
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
	auto io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    return window;
}

void Window::swap_buffers() {
	glfwSwapBuffers(this->glfw_window);
}