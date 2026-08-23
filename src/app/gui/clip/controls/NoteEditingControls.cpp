// NoteEditingControls.cpp
#include "NoteEditingControls.hpp"
#include "App.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace gsr::gui {

struct StrumNoteInfo {
    Model::Note* note;
    uint64_t orig_start_tick;
};

void NoteEditingControls::HandleKeyboardShortcuts(NoteEditorContext& ctx) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;

    auto& clip = ctx.clip;
    uint32_t grid_snap_ticks = ctx.grid_snap_ticks;

    if (!io.KeyCtrl) {
        // --- Glissando / Strumming / Snapped Arps (Shift + G) ---
        struct StrumState {
            std::vector<StrumNoteInfo> notes;
            int64_t step_ticks = 0;
        };
        static ModalSession<StrumState> s_strum;

        if (s_strum.Begin(io.KeyShift && ImGui::IsKeyDown(ImGuiKey_G))) {
            auto& s = s_strum.data;

            if (s_strum.just_started) {
                for (auto& note : clip.notes) {
                    if (note.selected) {
                        s.notes.push_back({&note, note.start_tick});
                    }
                }
                if (s.notes.size() <= 1) {
                    s_strum.Cancel();
                    return;
                }

                ctx.app.SaveUndoPoint();
                std::sort(s.notes.begin(), s.notes.end(), [](const auto& a, const auto& b) {
                    return a.note->pitch < b.note->pitch;
                });
            }

            int64_t step_delta = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) step_delta += grid_snap_ticks;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  step_delta -= grid_snap_ticks;

            if (io.MouseWheel != 0.0f) {
                int64_t scroll = static_cast<int64_t>(io.MouseWheel * (grid_snap_ticks / 4.0f));
                step_delta += (scroll != 0) ? scroll : (io.MouseWheel > 0 ? 10 : -10);
            }

            if (step_delta != 0) {
                s.step_ticks += step_delta;
                uint64_t max_start = clip.duration > 0 ? clip.duration - 1 : 0;
                for (size_t i = 0; i < s.notes.size(); ++i) {
                    int64_t new_start = static_cast<int64_t>(s.notes[i].orig_start_tick) + (i * s.step_ticks);
                    s.notes[i].note->start_tick = std::clamp<int64_t>(new_start, 0, max_start);
                }
            }

            return;
        }

        // --- Move Selection (Shift + T) ---
        struct MoveState {
            bool undo_saved = false;
        };
        static ModalSession<MoveState> s_move;

        if (s_move.Begin(io.KeyShift && ImGui::IsKeyDown(ImGuiKey_T))) {
            bool up    = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
            bool down  = ImGui::IsKeyPressed(ImGuiKey_DownArrow);
            bool left  = ImGui::IsKeyPressed(ImGuiKey_LeftArrow);
            bool right = ImGui::IsKeyPressed(ImGuiKey_RightArrow);

            if (up || down || left || right) {
                if (!s_move.data.undo_saved) {
                    ctx.app.SaveUndoPoint();
                    s_move.data.undo_saved = true;
                }

                uint64_t max_start = clip.duration > 0 ? clip.duration - 1 : 0;

                for (auto& note : clip.notes) {
                    if (!note.selected) continue;

                    if (up)   note.pitch = static_cast<uint8_t>(std::clamp(note.pitch + 1, 0, 127));
                    if (down) note.pitch = static_cast<uint8_t>(std::clamp(note.pitch - 1, 0, 127));

                    if (left) {
                        note.start_tick = (note.start_tick >= grid_snap_ticks) ? note.start_tick - grid_snap_ticks : 0;
                    }
                    if (right) {
                        note.start_tick = std::min(max_start, note.start_tick + grid_snap_ticks);
                    }
                }
            }
            return;
        }

        // --- Scale Duration/Pitch (Shift + S) ---
        struct ScaleState {
            bool undo_saved = false;
        };
        static ModalSession<ScaleState> s_scale;

        if (s_scale.Begin(io.KeyShift && ImGui::IsKeyDown(ImGuiKey_S))) {
            bool up    = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
            bool down  = ImGui::IsKeyPressed(ImGuiKey_DownArrow);
            bool left  = ImGui::IsKeyPressed(ImGuiKey_LeftArrow);
            bool right = ImGui::IsKeyPressed(ImGuiKey_RightArrow);

            if (up || down || left || right) {
                if (!s_scale.data.undo_saved) {
                    ctx.app.SaveUndoPoint();
                    s_scale.data.undo_saved = true;
                }

                for (auto& note : clip.notes) {
                    if (!note.selected) continue;

                    if (up)   note.pitch = static_cast<uint8_t>(std::clamp(note.pitch + 12, 0, 127));
                    if (down) note.pitch = static_cast<uint8_t>(std::clamp(note.pitch - 12, 0, 127));

                    if (left) {
                        note.duration = (note.duration > grid_snap_ticks) ? note.duration - grid_snap_ticks : grid_snap_ticks;
                    }
                    if (right) {
                        note.duration += grid_snap_ticks;
                    }
                }
            }
            return;
        }
    }
}

} // namespace gsr::gui