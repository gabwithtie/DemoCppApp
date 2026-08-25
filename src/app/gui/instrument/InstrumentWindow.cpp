#include "InstrumentWindow.hpp"
#include "FileDialogue.hpp"
#include <filesystem>

namespace gsr::gui
{

    InstrumentWindow::InstrumentWindow(gsr::App &app) : m_app(app) {}

    Model::Track *InstrumentWindow::GetSelectedTrack()
    {
        int track_idx = m_app.view.active_track_index;
        if (m_app.view.cell_selection.active)
        {
            track_idx = m_app.view.cell_selection.track_index;
        }
        if (track_idx >= 0 && track_idx < static_cast<int>(m_app.project.tracks.size()))
        {
            return &m_app.project.tracks[track_idx];
        }
        return nullptr;
    }

    void InstrumentWindow::DrawSelf()
    {
        Model::Track *track = GetSelectedTrack();
        if (!track)
        {
            ImGui::TextDisabled("No track selected.");
            return;
        }

        ImGui::Text("Track: %s", track->name.c_str());
        if (m_app.view.cell_selection.active)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(Bar %u)", m_app.view.cell_selection.start_bar + 1);
        }
        ImGui::Separator();

        // --- Active Instrument Engine Selection ---
        if (ImGui::CollapsingHeader("Instrument Assignment", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char *engines[] = {"SoundFont Synthesizer", "Sampler", "VST3 Plugin..."};
            int current_engine = (track->instrument_type == "Sampler") ? 1 : (track->instrument_type == "VST3 Plugin...") ? 2
                                                                                                                          : 0;

            if (ImGui::Combo("Engine", &current_engine, engines, IM_ARRAYSIZE(engines)))
            {
                track->instrument_type = engines[current_engine];
                track->needs_reload = true;
            }

            // --- SoundFont Specific UI Controls ---
            if (track->instrument_type == "SoundFont Synthesizer")
            {
                ImGui::Spacing();
                ImGui::Text("SoundFont (.sf2) File:");

                // Path Display & File Browser Button
                std::string filename = track->soundfont_path.empty() ? "No file loaded" : std::filesystem::path(track->soundfont_path).filename().string();
                
                char buf[256];
                snprintf(buf, sizeof(buf), "%s", filename.c_str());

                ImGui::InputText("##SF2Path", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                if (ImGui::Button("Browse...##SF2"))
                {
                    std::string selected = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN, "sf2");
                    if (!selected.empty() && selected != track->soundfont_path)
                    {
                        track->soundfont_path = selected;
                        track->preset_index = 0;
                        track->needs_reload = true;
                    }
                }

                // Preset & Gain Tweaks
                if (ImGui::SliderInt("Preset Index", &track->preset_index, 0, 127))
                {
                    track->needs_reload = true;
                }

                ImGui::SliderFloat("Output Gain", &track->instrument_gain, 0.0f, 2.0f, "%.2fx");
            }
        }
    }

} // namespace gsr::gui