#include "NoteSelectInteraction.hpp"
#include <algorithm>
#include <cmath>

namespace gsr::gui {

NoteSelectInteraction::NoteSelectInteraction()
    : m_controls{
        &m_controls_CtrlControls,
        &m_controls_NoteEditingControls,
        &m_controls_NoteSelection,
        &m_controls_NoteChordControls
      } {}

void NoteSelectInteraction::HandleKeyboardShortcuts(NoteEditorContext& ctx) {
    for (auto* control : m_controls) {
        control->HandleKeyboardShortcuts(ctx);
    }
}

void NoteSelectInteraction::ProcessSelect(
    gsr::App& app,
    Model::Clip& clip,
    ImVec2 mouse_pos,
    ImVec2 grid_origin,
    float& px_per_tick,
    float note_height,
    uint32_t grid_snap_ticks,
    bool canvas_hovered,
    int hovered_note_idx,
    bool edge_hovered
) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // 0. Construct frame context for extensions
    NoteEditorContext ctx{ app, clip, m_note_clipboard, grid_snap_ticks };

    float clip_px_width = clip.duration * px_per_tick;
    float rel_x = std::max(0.0f, mouse_pos.x - grid_origin.x);
    float rel_y = std::max(0.0f, mouse_pos.y - grid_origin.y);
    uint64_t raw_hover_tick = static_cast<uint64_t>(rel_x / px_per_tick);
    uint64_t snapped_hover_tick = (raw_hover_tick / grid_snap_ticks) * grid_snap_ticks;
    int hover_pitch = std::clamp(127 - static_cast<int>(rel_y / note_height), 0, 127);

    // Dispatch keyboard shortcuts
    HandleKeyboardShortcuts(ctx);

    if (edge_hovered || m_is_resizing) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    // 1. Double Click Note Creation
    if (canvas_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered_note_idx < 0) {
        if (rel_x <= clip_px_width && snapped_hover_tick < clip.duration) {
            app.SaveUndoPoint();
            Model::Note new_note;
            new_note.pitch = static_cast<uint8_t>(hover_pitch);
            new_note.start_tick = snapped_hover_tick;
            new_note.duration = grid_snap_ticks;
            new_note.velocity = 100;
            new_note.selected = true;
            clip.notes.push_back(new_note);
        }
    }
    // 2. Empty Space Click (Start Box Selection & Move Playhead)
    else if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_note_idx < 0 && !io.KeyAlt) {
        app.transport.current_tick = clip.start_tick + snapped_hover_tick;
        m_is_box_selecting = true;
        m_box_select_start = mouse_pos;

        if (!io.KeyShift && !io.KeyCtrl) {
            for (auto& note : clip.notes) note.selected = false;
        }
    }

    // 3. Note Select / Drag / Duplicate Handler
    if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_note_idx >= 0) {
        m_active_note_idx = hovered_note_idx;
        m_drag_mouse_start = mouse_pos;
        m_selected_initial_states.clear();

        if (!io.KeyShift && !io.KeyCtrl && !clip.notes[hovered_note_idx].selected) {
            for (auto& n : clip.notes) n.selected = false;
            clip.notes[hovered_note_idx].selected = true;
        } else if (io.KeyShift || io.KeyCtrl) {
            clip.notes[hovered_note_idx].selected = !clip.notes[hovered_note_idx].selected;
        }

        if (io.KeyAlt && !edge_hovered) {
            std::vector<Model::Note> duplicates;
            for (auto& n : clip.notes) {
                if (n.selected) {
                    n.selected = false;
                    Model::Note dup = n;
                    dup.selected = true;
                    duplicates.push_back(dup);
                }
            }
            for (const auto& dup : duplicates) clip.notes.push_back(dup);
        }

        if (edge_hovered) m_is_resizing = true;
        else m_is_dragging = true;

        for (size_t i = 0; i < clip.notes.size(); ++i) {
            if (clip.notes[i].selected) {
                m_selected_initial_states.push_back({i, clip.notes[i].start_tick, clip.notes[i].duration, clip.notes[i].pitch});
            }
        }
    }

    // 4. Box Selection Active State
    if (m_is_box_selecting && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        app.transport.current_tick = clip.start_tick + snapped_hover_tick;

        ImVec2 box_min(std::min(m_box_select_start.x, mouse_pos.x), std::min(m_box_select_start.y, mouse_pos.y));
        ImVec2 box_max(std::max(m_box_select_start.x, mouse_pos.x), std::max(m_box_select_start.y, mouse_pos.y));

        draw_list->AddRectFilled(box_min, box_max, IM_COL32(100, 180, 255, 40));
        draw_list->AddRect(box_min, box_max, IM_COL32(120, 200, 255, 200), 0.0f, 0, 1.5f);

        bool append_mode = io.KeyShift || io.KeyCtrl;
        for (auto& note : clip.notes) {
            float nx1 = grid_origin.x + (note.start_tick * px_per_tick);
            float nx2 = nx1 + (note.duration * px_per_tick);
            float ny1 = grid_origin.y + (127 - note.pitch) * note_height;
            float ny2 = ny1 + note_height;

            bool intersects = (nx1 < box_max.x && nx2 > box_min.x && ny1 < box_max.y && ny2 > box_min.y);
            note.selected = append_mode ? (note.selected || intersects) : intersects;
        }
    }

    // 5. Active Group Drag & Resize
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !m_selected_initial_states.empty()) {
        float delta_x = mouse_pos.x - m_drag_mouse_start.x;
        float delta_y = mouse_pos.y - m_drag_mouse_start.y;

        int64_t delta_ticks = static_cast<int64_t>(delta_x / px_per_tick);
        int64_t snapped_delta_ticks = (delta_ticks / static_cast<int64_t>(grid_snap_ticks)) * grid_snap_ticks;
        int delta_pitch = -static_cast<int>(std::round(delta_y / note_height));

        if (m_is_resizing) {
            for (const auto& st : m_selected_initial_states) {
                if (st.index < clip.notes.size()) {
                    int64_t new_dur = std::max<int64_t>(grid_snap_ticks, static_cast<int64_t>(st.duration) + snapped_delta_ticks);
                    clip.notes[st.index].duration = static_cast<uint64_t>(new_dur);
                }
            }
        } else if (m_is_dragging) {
            int64_t min_initial_tick = m_selected_initial_states[0].start_tick;
            for (const auto& st : m_selected_initial_states) {
                min_initial_tick = std::min<int64_t>(min_initial_tick, st.start_tick);
            }
            int64_t allowed_delta_ticks = std::max(-min_initial_tick, snapped_delta_ticks);

            for (const auto& st : m_selected_initial_states) {
                if (st.index < clip.notes.size()) {
                    int64_t target_tick = std::max<int64_t>(0, static_cast<int64_t>(st.start_tick) + allowed_delta_ticks);
                    int target_pitch = std::clamp<int>(static_cast<int>(st.pitch) + delta_pitch, 0, 127);

                    clip.notes[st.index].start_tick = static_cast<uint64_t>(target_tick);
                    clip.notes[st.index].pitch = static_cast<uint8_t>(target_pitch);
                }
            }
        }
    }

    // 6. Release Drag / Box Selection
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_active_note_idx = -1;
        m_is_resizing = false;
        m_is_dragging = false;
        m_is_box_selecting = false;
        m_selected_initial_states.clear();
    }

    // Context Menu
    if (ImGui::BeginPopupContextWindow("NoteManagerContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
        for (auto* control : m_controls) {
            control->DrawContextMenu(ctx);
        }
        ImGui::EndPopup();
    }
}

} // namespace gsr::gui