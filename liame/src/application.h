#pragma once
#include <GLFW/glfw3.h>
#include "core.h"
#include <vector>
#include "window.h"
#include "memory.h"

#include "spdlog/spdlog.h"

#define NUMBER_OF_KEYS 349
#define NUMBER_OF_MOUSE_BUTTONS 8

struct Application;
struct Layer;
struct Event;

enum class Action {
	NOTHING = 0,
	DOWN,
	UP,
	PRESS
};

enum class Key {
	K_UNKNOWN = -1,
	K_SPACE = 32,
	K_APOSTROPHE = 39,
	K_COMMA = 44,
	K_MINUS = 45,
	K_PERIOD = 46,
	K_SLASH = 47,
	K_NR_0 = 48,
	K_NR_1 = 49,
	K_NR_2 = 50,
	K_NR_3 = 51,
	K_NR_4 = 52,
	K_NR_5 = 53,
	K_NR_6 = 54,
	K_NR_7 = 55,
	K_NR_8 = 56,
	K_NR_9 = 57,
	K_SEMICOLON = 59,
	K_EQUAL = 61,
	K_A = 65,
	K_B = 66,
	K_C = 67,
	K_D = 68,
	K_E = 69,
	K_F = 70,
	K_G = 71,
	K_H = 72,
	K_I = 73,
	K_J = 74,
	K_K = 75,
	K_L = 76,
	K_M = 77,
	K_N = 78,
	K_O = 79,
	K_P = 80,
	K_Q = 81,
	K_R = 82,
	K_S = 83,
	K_T = 84,
	K_U = 85,
	K_V = 86,
	K_W = 87,
	K_X = 88,
	K_Y = 89,
	K_Z = 90,
	K_LEFT_BACKET = 91,
	K_BACKSLASH = 92,
	K_RIGHT_BRACKET = 93,
	K_GRAVE_ACCENT = 96,
	K_WORLD_1 = 161,
	K_WORLD_2 = 162,
	K_ESCAPE = 256,
	K_ENTER = 257,
	K_TAB = 258,
	K_BACKSPACE = 259,
	K_INSERT = 260,
	K_DELETE = 261,
	K_RIGHT = 262,
	K_LEFT = 263,
	K_DOWN = 264,
	K_UP = 265,
	K_PAGE_UP = 266,
	K_PAGE_DOWN = 267,
	K_HOME = 268,
	K_END = 269,
	K_CAPS_LOCK = 280,
	K_SCROLL_LOCK = 281,
	K_NUM_LOCK = 282,
	K_PRs32_SCREEN = 283,
	K_PAUSE = 284,
	K_F1 = 290,
	K_F2 = 291,
	K_F3 = 292,
	K_F4 = 293,
	K_F5 = 294,
	K_F6 = 295,
	K_F7 = 296,
	K_F8 = 297,
	K_F9 = 298,
	K_F10 = 299,
	K_F11 = 300,
	K_F12 = 301,
	K_F13 = 302,
	K_F14 = 303,
	K_F15 = 304,
	K_F16 = 305,
	K_F17 = 306,
	K_F18 = 307,
	K_F19 = 308,
	K_F20 = 309,
	K_F21 = 310,
	K_F22 = 311,
	K_F23 = 312,
	K_F24 = 313,
	K_F25 = 314,
	K_KP_0 = 320,
	K_KP_1 = 321,
	K_KP_2 = 322,
	K_KP_3 = 323,
	K_KP_4 = 324,
	K_KP_5 = 325,
	K_KP_6 = 326,
	K_KP_7 = 327,
	K_KP_8 = 328,
	K_KP_9 = 329,
	K_KP_DECIMAL = 330,
	K_KP_DIVIDE = 331,
	K_KP_MULTIPLY = 332,
	K_KP_SUBTRACT = 333,
	K_KP_ADD = 334,
	K_KP_ENTER = 335,
	K_KP_EQUAL = 336,
	K_LEFT_SHIFT = 340,
	K_LEFT_CONTROL = 341,
	K_LEFT_ALT = 342,
	K_LEFT_SUPER = 343,
	K_RIGHT_SHIFT = 344,
	K_RIGHT_CONTROL = 345,
	K_RIGHT_ALT = 346,
	K_RIGHT_SUPER = 347,
	K_MENU = 348,
};

enum class MouseButton {
	M_UNKNOWN = -1,
	M_MOUSE_1 = 0,
	M_MOUSE_2 = 1,
	M_MOUSE_3 = 2,
	M_MOUSE_4 = 3,
	M_MOUSE_5 = 4,
	M_MOUSE_6 = 5,
	M_MOUSE_7 = 6,
	M_MOUSE_8 = 7,
};

struct WindowResizeEvent {
	s64 width;
	s64 height;
};

struct MouseMoveEvent {
	s64 x;
	s64 y;
};

struct MouseInputEvent {
	MouseButton button;
	Action action;
};

struct KeyboardInputEvent {
	Key key;
	Action action;
	bool shift;	
	bool ctrl;
	bool alt;	
	bool super;	
	bool caps_lock; 	
	bool num_lock;	
};

enum class EventType {
	INPUT = 0,
	MOUSE_MOVE,
	MOUSE_INPUT,
	KEYBOARD_INPUT,
	WINDOW_RESIZE,
};

struct Event {
	EventType  type;
	union {
		MouseMoveEvent mouse_move;
		MouseInputEvent mouse_input;
		KeyboardInputEvent keyboard_input;
		WindowResizeEvent window_resize;
	};
};

struct Input {
	Action mouse_buttons[NUMBER_OF_MOUSE_BUTTONS];
	double mouse_x;
	double mouse_y;
	Action keys[NUMBER_OF_KEYS];
	bool shift;
	bool ctrl;
	bool alt;
	bool super;
	bool caps_lock;
	bool num_lock;
};

struct Application {
	Window window;
	bool is_running;
	f64 delta_time;
	f64 time;
	std::vector<Layer*> layers;
	
	void start();
	void draw();
	void update();
	void close();
	void send_event(Event* ev);
	~Application();
};

SmartPointer<Application> make_application(s32 width, s32 height, const char* title);

bool key_action(Key key, Action action);
bool mouse_action(MouseButton button, Action action);
void reset_input();

struct Layer {
    virtual void start(Application* application)                    = 0;
    virtual void update(Application* application)                   = 0;
    virtual void render(Application* application)                   = 0;
    virtual void ui(Application* application)                       = 0;
    virtual void end(Application* application)                      = 0;
    virtual bool send_event(Application* application, Event* ev)    = 0;
	virtual ~Layer() = default;
};

void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods);
void mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods);
void cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void window_size_callback(GLFWwindow* window, s32 width, s32 height);