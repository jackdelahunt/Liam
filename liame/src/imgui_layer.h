#pragma once    

#include "application.h"

struct ImGuiLayer : public Layer {
    void start(Application* application)                    override;
    void update(Application* application)                   override;
    void render(Application* application)                   override;
    void ui(Application* application)                       override;
    void end(Application* application)                      override;
    bool send_event(Application* application, Event* ev)    override;
};