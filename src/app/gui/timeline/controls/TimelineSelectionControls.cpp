#include "TimelineSelectionControls.hpp"
#include <algorithm>

namespace gsr::gui {

void TimelineSelectionControls::HandleKeyboardShortcuts(TimelineEditorContext& ctx) {
    ImGuiIO& io = ImGui::GetIO();
    auto& sel = ctx.app.view.cell_selection;

    if (!sel.active) return;

    bool shift = io.KeyShift;

    // --- Horizontal Navigation & Widening (Left / Right) ---
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (shift) {
            // Widen or contract selection to the left
            if (sel.num_bars > 1 && sel.drag_anchor_bar < sel.start_bar + sel.num_bars - 1) {
                sel.num_bars--;
            } else if (sel.start_bar > 0) {
                sel.start_bar--;
                sel.num_bars++;
            }
        } else {
            // Move 1 bar left
            if (sel.start_bar > 0) {
                sel.start_bar--;
            }
            sel.num_bars = 1;
            sel.drag_anchor_bar = sel.start_bar;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (shift) {
            // Widen selection to the right
            sel.num_bars++;
        } else {
            // Move 1 bar right
            sel.start_bar++;
            sel.num_bars = 1;
            sel.drag_anchor_bar = sel.start_bar;
        }
    }

    // --- Vertical Track Navigation (Up / Down) ---
    int num_tracks = static_cast<int>(ctx.app.project.tracks.size());
    if (num_tracks == 0) return;

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        int new_idx = std::max(0, sel.track_index - 1);
        sel.track_index = new_idx;
        ctx.app.view.active_track_index = new_idx;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        int new_idx = std::min(num_tracks - 1, sel.track_index + 1);
        sel.track_index = new_idx;
        ctx.app.view.active_track_index = new_idx;
    }
}

} // namespace gsr::gui