#pragma once

#include <GLFW/glfw3.h>
#include "core.h"

struct Window {
    GLFWwindow* glfw_window;
	s32 width;
	s32 height;
	const char* title;
	
	void swap_buffers();
};

Window make_window(
	s32 width, s32 height, 
	const char* title, 
	void (*key_callback)(GLFWwindow*, s32, s32, s32, s32),
	void (*mouse_button_callback)(GLFWwindow*, s32, s32, s32),
	void (*cursor_position_callback)(GLFWwindow*, f64, f64),
	void (*window_size_callback)(GLFWwindow*, s32, s32)
);
