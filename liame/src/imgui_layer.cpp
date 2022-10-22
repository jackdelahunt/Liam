#include <GLFW/glfw3.h>
#include "imgui_layer.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "application.h"


void ImGuiLayer::start(Application* application)                    {}
void ImGuiLayer::update(Application* application)                   {}
void ImGuiLayer::render(Application* application)                   {}
void ImGuiLayer::ui(Application* application)                       {
#if 0
	bool draw = true;
	ImGui::ShowDemoWindow(&draw);
#endif

    // Info window
    ImGui::Begin("Ram info window");
    ImGui::Text("Time since start %.1f", application->time);
    ImGui::Text("FPS %.1f", 1 / application->delta_time);
    ImGui::Text("Viewport width %d", application->window.width);
    ImGui::Text("Viewport height %d", application->window.width);
    ImGui::Text("Layer count %d", application->layers.size());
    ImGui::End();
}
void ImGuiLayer::end(Application* application)                      {}
bool ImGuiLayer::send_event(Application* application, Event* ev)    {return false;}