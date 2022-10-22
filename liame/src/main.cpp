#include <ctype.h>
#include <stdio.h>
#include <engine.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

int main() {

	EDITOR_START();

	LOG_DEBUG("Hello world");
	LOG_INFO("Welcome to the game");

	SmartPointer<Application> application = make_application(1000, 720, "The Ram window");
	
	application->start();
	while (application->is_running) {
		application->update();
		application->draw();
	}
}