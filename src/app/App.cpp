#include "App.hpp"
#include "ProjectLoader.hpp"
#include "FileDialogue.hpp"
#include <algorithm>
#include <imgui.h>

namespace gsr {

App::App() = default;
App::~App() = default;

void App::process_input() {
    ImGuiIO& io = ImGui::GetIO();

    if (!io.WantTextInput) {
        // Spacebar Play / Pause
        if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
            if (transport.state == PlaybackState::Playing) {
                transport.state = PlaybackState::Paused;
            } else {
                if (view.cell_selection.active) {
                    uint32_t ticks_per_bar = project.ppq * 4;
                    transport.current_tick = static_cast<uint64_t>(view.cell_selection.start_bar) * ticks_per_bar;
                }
                transport.state = PlaybackState::Playing;
            }
        }

        // Quick Save (Ctrl + S)
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (!ProjectLoader::QuickSave()) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "gsrproj");
                if (!outPath.empty()) {
                    ProjectLoader::SaveProject(outPath);
                }
            }
        }

        // Undo (Ctrl + Z) / Redo (Ctrl + Y or Ctrl + Shift + Z)
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
            if (io.KeyShift) {
                Redo();
            } else {
                Undo();
            }
        } else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
            Redo();
        }
    }
}

bool App::init() {
    s_instance = this;
    is_running = true;

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