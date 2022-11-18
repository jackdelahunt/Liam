#include <ctype.h>
#include <stdio.h>
#include <engine.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

f32 verts[20] = {
	-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		// bot left
	0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		// bot right
	0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		// top right
	-0.5f, 0.5f, 0.0f,		0.0f, 1.0f		// top left
};

s32 indices[6] = {
	0, 1, 2, // right triangle
	0, 2, 3 // left triangle
};

int main() {

	ENGINE_START();

	LOG_DEBUG_ENGINE("Hello world");
	LOG_INFO("Welcome to the game");

	SmartPointer<Application> application = make_application(1000, 720, "The Ram window");
	
	application->start();
	while (application->is_running) {
		application->update();
		application->draw();
	}
}