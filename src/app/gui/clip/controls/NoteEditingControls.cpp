#include "NoteEditingControls.hpp"
#include "gui/clip/NoteManager.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace gsr::gui {

struct StrumNoteInfo {
    Model::Note* note;
    uint64_t orig_start_tick;
};

void NoteEditingControls::HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip) {
    ImGuiIO& io = ImGui::GetIO();
    uint32_t grid_snap_ticks = m_note_manager->GetGridSnapTicks();

    if (!io.KeyCtrl) {
        // --- Glissando / Strumming / Snapped Arps (Shift + G) ---
        static bool s_is_strumming = false;
        static std::vector<StrumNoteInfo> s_strum_notes;
        static int64_t s_step_ticks = 0;

        bool is_strum_shortcut_held = io.KeyShift && ImGui::IsKeyDown(ImGuiKey_G);

        if (is_strum_shortcut_held) {
            // Intercept mouse wheel input to stop the timeline from scrolling sideways
            float wheel_delta = io.MouseWheel;

            // Initialize strum session and cache baseline positions
            if (!s_is_strumming) {
                s_strum_notes.clear();
                for (auto& note : clip.notes) {
                    if (note.selected) {
                        s_strum_notes.push_back({ &note, note.start_tick });
                    }
                }

                if (s_strum_notes.size() > 1) {
                    app.SaveUndoPoint();
                    s_is_strumming = true;
                    s_step_ticks = 0;

                    // Sort ascending by pitch (lowest note stays fixed at its original start tick)
                    std::sort(s_strum_notes.begin(), s_strum_notes.end(), [](const StrumNoteInfo& a, const StrumNoteInfo& b) {
                        return a.note->pitch < b.note->pitch;
                    });
                }
            }

            // Adjust strum offsets
            if (s_is_strumming && !s_strum_notes.empty()) {
                bool modified = false;

                // Left/Right Arrow Keys for Snapped Arpeggios (1 grid unit per press)
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                    s_step_ticks += grid_snap_ticks;
                    modified = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                    s_step_ticks -= grid_snap_ticks;
                    modified = true;
                }

                // Mouse Scroll for fine Glissando
                if (wheel_delta != 0.0f) {
                    int64_t scroll_step = static_cast<int64_t>(wheel_delta * (grid_snap_ticks / 4));
                    if (scroll_step == 0) {
                        scroll_step = wheel_delta > 0.0f ? 10 : -10;
                    }
                    s_step_ticks += scroll_step;
                    modified = true;
                }

                // Apply offset to cached base positions
                if (modified) {
                    uint64_t max_start = clip.duration > 0 ? clip.duration - 1 : 0;
                    for (size_t i = 0; i < s_strum_notes.size(); ++i) {
                        int64_t offset = static_cast<int64_t>(i) * s_step_ticks;
                        int64_t new_start = static_cast<int64_t>(s_strum_notes[i].orig_start_tick) + offset;
                        s_strum_notes[i].note->start_tick = static_cast<uint64_t>(std::clamp<int64_t>(new_start, 0, max_start));
                    }
                }
            }
            return; // Block standard arrow key actions while holding Shift + G
        } else {
            // Reset state when releasing Shift + G
            if (s_is_strumming) {
                s_is_strumming = false;
                s_strum_notes.clear();
                s_step_ticks = 0;
            }
        }

        // Pitch Shift Up / Down (1 Semitone or 1 Octave with Shift)
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            app.SaveUndoPoint();

            int shift = io.KeyShift ? 12 : 1;
            for (auto& note : clip.notes) {
                if (note.selected) {
                    note.pitch = static_cast<uint8_t>(std::clamp(note.pitch + shift, 0, 127));
                }
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            app.SaveUndoPoint();

            int shift = io.KeyShift ? 12 : 1;
            for (auto& note : clip.notes) {
                if (note.selected) {
                    note.pitch = static_cast<uint8_t>(std::clamp(note.pitch - shift, 0, 127));
                }
            }
        }

        // Horizontal Shift vs Duration Stretch (Left / Right)
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            app.SaveUndoPoint();

            for (auto& note : clip.notes) {
                if (note.selected) {
                    if (io.KeyShift) {
                        // Shorten duration by one grid snap
                        if (note.duration > grid_snap_ticks) {
                            note.duration -= grid_snap_ticks;
                        } else {
                            note.duration = grid_snap_ticks;
                        }
                    } else {
                        // Move start position left by one grid snap
                        if (note.start_tick >= grid_snap_ticks) {
                            note.start_tick -= grid_snap_ticks;
                        } else {
                            note.start_tick = 0;
                        }
                    }
                }
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            app.SaveUndoPoint();

            for (auto& note : clip.notes) {
                if (note.selected) {
                    if (io.KeyShift) {
                        // Extend duration by one grid snap
                        note.duration += grid_snap_ticks;
                    } else {
                        // Move start position right by one grid snap
                        uint64_t max_start = clip.duration > 0 ? clip.duration - 1 : 0;
                        note.start_tick = std::min(max_start, note.start_tick + grid_snap_ticks);
                    }
                }
            }
        }
    }
}

} // namespace gsr::gui