#include <GLFW/glfw3.h>
#include <stdio.h>
#include "application.h"
#include "memory.h"
#include "imgui_layer.h"
#include "memory.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

Input input = {};

SmartPointer<Application> make_application(s32 width, s32 height, const char* title) {

	Application *application = new Application {
		.window = NULL,
		.is_running = true,
		.delta_time = 0.0f,
		.time = 0.0,
		.layers = std::vector<Layer *>(),
	};

	application->window = make_window(
		width, height, 
		title, 
		key_callback, 
		mouse_button_callback, 
		cursor_position_callback, 
		window_size_callback
	);

	// when ever i get user pointer for this window
	// I can then acces the Window pointer that owns it
	glfwSetWindowUserPointer(application->window.glfw_window, application);


	// add imgui layer to the application
	ImGuiLayer* imgui_layer = new ImGuiLayer();
	application->layers.push_back(imgui_layer);

	return make_smart_pointer(application);
}

void Application::start() {
	for(s32 i = 0; i < this->layers.size(); i++) {
		this->layers.at(i)->start(this);
	}
}

void Application::draw() {
	if (!this->is_running) return;

	// tell imgui to start preparing to draw another frame
	// this is done before any imgui ui calls and is done once
	// per frame. once all ui is draw we then render imgui 
	auto io = &ImGui::GetIO();
	io->DisplaySize = ImVec2((f32)this->window.width, (f32)this->window.height);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	// draw from last layer all the way back up,
	// this makes each layer be drawn in order
	for(s32 i = this->layers.size() - 1; i >= 0; i--) {
		Layer* layer = this->layers.at(i);
		
		layer->render(this);
		layer->ui(this);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// updates for windows as they are outside the viewport
	// gets the current context and sets it back as update 
	// might change it... shuold bring it back to application
	// window that we have
	GLFWwindow* backup_current_context = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(backup_current_context);
	
	this->window.swap_buffers();
}

void Application::update() {
	if (!this->is_running) return;

	// set current time and set the delta time to last frame
	f64 this_time = glfwGetTime();
	this->delta_time = this_time - this->time;
	this->time = this_time;

	if(input.keys[(u32)Key::K_ESCAPE] == Action::DOWN) {
		this->close();
	}

	reset_input();

	glfwPollEvents();

	for(s32 i = 0; i < this->layers.size(); i++) {
		this->layers.at(i)->update(this);
	}
}

void Application::close() {
	// shutting down process of the application,
	// call end on each layer then terminate windowing
	for(s32 i = 0; i < this->layers.size(); i++) {
		this->layers.at(i)->end(this);
	}
	glfwDestroyWindow(this->window.glfw_window);
	glfwTerminate();
	this->is_running = false;
}

void Application::send_event(Event* ev) {
	for(s32 i = 0; i < this->layers.size(); i++) {
		if(this->layers.at(i)->send_event(this, ev)) break;
	}
}

Application::~Application()
{
	for(Layer *layer : this->layers)
	{
		delete layer;
	}	
}

bool key_action(Key key, Action action)
{
	return input.keys[(s32)key] == action;
}

bool mouse_action(MouseButton button, Action action)
{
	return  input.mouse_buttons[(s32)button] == action;	
}

void reset_input() {
	// reset all key inputs back to false for this frame
	for (s32 i = 0; i < NUMBER_OF_KEYS; i++) {
		input.keys[i] = Action::NOTHING;
	}

	for (s32 i = 0; i < NUMBER_OF_MOUSE_BUTTONS; i++) {
		input.mouse_buttons[i] = Action::NOTHING;
	}

	input.shift 		= false;
	input.ctrl 		= false;
	input.alt 		= false;
	input.super 		= false;
	input.caps_lock 	= false;
	input.caps_lock 	= false;
	input.num_lock 	= false;
}

void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods) {
	Application* application = (Application*)glfwGetWindowUserPointer(window);

	// this is for setting inputs in the application
	// used for basic checking of input from the game
	if (action == GLFW_PRESS) {
		input.keys[key] = Action::DOWN;
	}
	else if (action == GLFW_RELEASE) {
		input.keys[key] = Action::UP;
	}
	else if (action == GLFW_REPEAT) {
		input.keys[key] = Action::PRESS;
	}
	else {
		input.keys[key] = Action::NOTHING;
	}

	// set mods input
	input.shift 		= (mods & GLFW_MOD_SHIFT) != 0;
	input.ctrl 		= (mods & GLFW_MOD_CONTROL) != 0;
	input.alt 			= (mods & GLFW_MOD_ALT) != 0;
	input.super 		= (mods & GLFW_MOD_SUPER) != 0;
	input.caps_lock 	= (mods & GLFW_MOD_CAPS_LOCK) != 0;
	input.num_lock 	= (mods & GLFW_MOD_NUM_LOCK) != 0;

	// this is for passing input event to the application
	// used for event based things like UI
	/// NOTE: currently only input from main keyboard
	// is sent as events... so ctrl or shift are not passed
	// to each layer, might be an issue
	Event ev = Event {
		.type = EventType::KEYBOARD_INPUT,
		.keyboard_input = KeyboardInputEvent {
			.key = (Key)key,
			.action = (Action)action,
			.shift = input.shift, 
			.ctrl = input.ctrl, 
			.alt = input.alt, 
			.super = input.super, 
			.caps_lock = input.caps_lock, 
			.num_lock = input.num_lock, 
		}
	};

	application->send_event(&ev);
}

void mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods) {
	Application* application = (Application*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS) {
		input.mouse_buttons[button] = Action::DOWN;
	}
	else if (action == GLFW_RELEASE) {
		input.mouse_buttons[button] = Action::UP;
	}
	else if (action == GLFW_REPEAT) {
		input.mouse_buttons[button] = Action::PRESS;
	}
	else {
		input.mouse_buttons[button] = Action::NOTHING;
	}

	// this is for passing input event to the application
	// used for event based things like UI
	Event ev = Event {
		.type = EventType::MOUSE_INPUT,
		.mouse_input = MouseInputEvent {
			.button = (MouseButton)button,
			.action = (Action)action
		}
	};

	application->send_event(&ev);
}

void cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	Application* application = (Application*)glfwGetWindowUserPointer(window);
	input.mouse_x = xpos;
	input.mouse_y = ypos;

	// this is for passing input event to the application
	// used for event based things like UI
	Event ev = Event {
		.type = EventType::MOUSE_MOVE,
		.mouse_move = MouseMoveEvent {
			.x = (s64) xpos,
			.y = (s64) ypos
		}
	};

	application->send_event(&ev);
}

void window_size_callback(GLFWwindow* window, s32 width, s32 height)
{
	Application* application = (Application*)glfwGetWindowUserPointer(window);
	application->window.width = width;
	application->window.width = height;

	// this is for passing input event to the application
	// used for event based things like UI
	Event ev = Event {
		.type = EventType::WINDOW_RESIZE,
		.window_resize = WindowResizeEvent {
			.width = width,
			.height = height,
		}
	};

	application->send_event(&ev);
}
