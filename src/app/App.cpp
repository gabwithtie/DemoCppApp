#include "App.hpp"
#include <algorithm>

namespace gsr {

App::App() = default;
App::~App() = default;

bool App::init() {
    s_instance = this;
    is_running = true;
    return true;
}

void App::process_input() {
    // Process global application hotkeys (e.g., Spacebar for Play/Pause)
}

void App::update(float delta_time) {
    if (transport.state != PlaybackState::Playing) {
        return;
    }

    transport.current_time_sec += delta_time;

    // Fetch active BPM (defaults to first tempo entry or fallback 120.0)
    double bpm = project.tempo_map.empty() ? 120.0 : project.tempo_map[0].bpm;

    // Calculate delta ticks: (BPM / 60) * PPQ * delta_seconds
    double ticks_per_second = (bpm / 60.0) * static_cast<double>(project.ppq);
    uint64_t elapsed_ticks = static_cast<uint64_t>(ticks_per_second * delta_time);
    
    transport.current_tick += elapsed_ticks;

    // Handle timeline looping logic
    if (transport.loop_enabled && transport.current_tick >= transport.loop_end_tick) {
        uint64_t loop_length = transport.loop_end_tick - transport.loop_start_tick;
        if (loop_length > 0) {
            uint64_t overshoot = (transport.current_tick - transport.loop_start_tick) % loop_length;
            transport.current_tick = transport.loop_start_tick + overshoot;
        } else {
            transport.current_tick = transport.loop_start_tick;
        }
    }
}

void App::render_ui() {
    // DockSpace submission and window frame calls (Piano Roll, Score, Inspector) go here
}

void App::shutdown() {
    is_running = false;
}


} // namespace gsr