#include "App.hpp"
#include <algorithm>

#include <imgui.h>

namespace gsr {

App::App() = default;
App::~App() = default;

void App::process_input() {
    ImGuiIO& io = ImGui::GetIO();

    // Trigger Play/Pause globally unless actively typing in a text input box
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
        if (transport.state == PlaybackState::Playing) {
            transport.state = PlaybackState::Paused;
        } else {
            // Jump to selection start if a bar selection is active
            if (view.cell_selection.active) {
                uint32_t ticks_per_bar = project.ppq * 4;
                transport.current_tick = static_cast<uint64_t>(view.cell_selection.start_bar) * ticks_per_bar;
            }
            transport.state = PlaybackState::Playing;
        }
    }
}

bool App::init() {
    s_instance = this;
    is_running = true;

    // Start Audio Engine & Callback Thread
    if (!audio_engine.Init(44100, 512)) {
        return false;
    }

    return true;
}

void App::update(float delta_time) {
    
}

void App::shutdown() {
    audio_engine.Shutdown();
    is_running = false;
}


} // namespace gsr