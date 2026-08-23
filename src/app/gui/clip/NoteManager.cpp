#include "NoteManager.hpp"
#include <algorithm>
#include <cmath>

namespace gsr::gui
{

    // --- Non-member helpers for Glaze aggregate compatibility ---
    static void EnsureNoteSegments(Model::Note &note, uint64_t seg_res)
    {
        size_t count = std::max<size_t>(1, (note.duration + seg_res - 1) / seg_res);
        if (note.paint_segments.size() != count)
        {
            float default_val = static_cast<float>(note.velocity) / 127.0f;
            note.paint_segments.resize(count, default_val);
        }
    }

    static void SyncVelocityFromSegments(Model::Note &note)
    {
        if (!note.paint_segments.empty())
        {
            note.velocity = static_cast<uint8_t>(std::clamp(note.paint_segments[0] * 127.0f, 0.0f, 127.0f));
        }
    }

    void NoteManager::HandleKeyboardShortcuts(gsr::App &app, Model::Clip &clip)
    {
        for (auto &control : this->m_controls)
        {
            control->HandleKeyboardShortcuts(app, clip);
        }
    }

    void NoteManager::DrawPlayhead(gsr::App &app, const Model::Clip &clip, ImVec2 grid_origin, ImVec2 grid_size, float px_per_tick, ImDrawList *draw_list)
    {
        int64_t rel_playhead_tick = static_cast<int64_t>(app.transport.current_tick) - static_cast<int64_t>(clip.start_tick);

        if (rel_playhead_tick >= 0)
        {
            float playhead_x = grid_origin.x + (rel_playhead_tick * px_per_tick);
            if (playhead_x >= grid_origin.x && playhead_x <= grid_origin.x + grid_size.x)
            {
                draw_list->AddLine(
                    ImVec2(playhead_x, grid_origin.y),
                    ImVec2(playhead_x, grid_origin.y + grid_size.y),
                    IM_COL32(255, 75, 75, 255),
                    2.0f);
            }
        }
    }

    void NoteManager::ProcessAndDrawNotes(
        gsr::App &app,
        Model::Clip &clip,
        ImVec2 grid_origin,
        ImVec2 grid_size,
        float &px_per_tick,
        float note_height,
        uint32_t grid_snap_ticks,
        bool canvas_hovered)
    {
        this->cache_grid_snap_ticks = grid_snap_ticks;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 mouse_pos = io.MousePos;

        float clip_px_width = clip.duration * px_per_tick;

        float rel_x = std::max(0.0f, mouse_pos.x - grid_origin.x);
        float rel_y = std::max(0.0f, mouse_pos.y - grid_origin.y);
        uint64_t raw_hover_tick = static_cast<uint64_t>(rel_x / px_per_tick);
        uint64_t snapped_hover_tick = (raw_hover_tick / grid_snap_ticks) * grid_snap_ticks;
        int hover_pitch = std::clamp(127 - static_cast<int>(rel_y / note_height), 0, 127);

        HandleKeyboardShortcuts(app, clip);

        // --- Piano Roll Local Navigation (Zoom & Pan) ---
        if (canvas_hovered)
        {
            // 1. Local Horizontal Zoom (Shift + Mouse Scroll)
            if (io.KeyShift && io.MouseWheel != 0.0f)
            {
                float zoom_speed = 0.05f;
                float zoom_factor = (io.MouseWheel > 0.0f) ? 1.0f + zoom_speed : 1.0f - zoom_speed;
                px_per_tick = std::clamp(px_per_tick * zoom_factor, 0.005f, 0.2f);
            }

            // 2. Local Horizontal Pan (Alt + Mouse Delta X)
            if (io.KeyAlt && io.MouseDelta.x != 0.0f && !m_is_dragging && !m_is_box_selecting)
            {
                float current_scroll_x = ImGui::GetScrollX();
                ImGui::SetScrollX(current_scroll_x - io.MouseDelta.x);
            }

            // Intercept scroll wheel events to stop default container scrolling
            io.MouseWheel = 0.0f;
            io.MouseWheelH = 0.0f;
        }

        // Hit Test Notes
        int hovered_note_idx = -1;
        bool edge_hovered = false;

        // Hit Test & Render Notes
        for (size_t i = 0; i < clip.notes.size(); ++i)
        {
            auto &note = clip.notes[i];
            EnsureNoteSegments(note, SEGMENT_TICK_RES);

            float nx1 = grid_origin.x + (note.start_tick * px_per_tick);
            float nx2 = nx1 + (note.duration * px_per_tick);
            float ny1 = grid_origin.y + (127 - note.pitch) * note_height;
            float ny2 = ny1 + note_height;

            ImVec2 n_min(nx1, ny1 + 1.0f);
            ImVec2 n_max(nx2, ny2 - 1.0f);

            // Alpha scaling driven by velocity / first paint segment
            float val_norm = note.paint_segments[0];
            int alpha = static_cast<int>(std::clamp(val_norm * 230.0f + 25.0f, 25.0f, 255.0f));

            ImU32 fill_col = note.selected ? IM_COL32(255, 210, 80, alpha) : IM_COL32(230, 150, 40, alpha);
            ImU32 border_col = note.selected ? IM_COL32(255, 255, 200, 255) : IM_COL32(255, 190, 100, alpha);

            draw_list->AddRectFilled(n_min, n_max, fill_col, 2.0f);
            draw_list->AddRect(n_min, n_max, border_col, 2.0f);

            if (mouse_pos.x >= n_min.x && mouse_pos.x <= n_max.x && mouse_pos.y >= n_min.y && mouse_pos.y <= n_max.y)
            {
                hovered_note_idx = static_cast<int>(i);
                if (mouse_pos.x >= n_max.x - 6.0f)
                    edge_hovered = true;
            }
        }

        // --- Mode Branching ---
        if (m_edit_mode == EditMode::Paint)
        {
            if (canvas_hovered)
            {
                // Render brush indicator
                draw_list->AddCircle(mouse_pos, m_brush_radius, IM_COL32(255, 255, 255, 180), 32, 1.5f);
                draw_list->AddCircleFilled(mouse_pos, m_brush_radius, IM_COL32(255, 255, 255, 20));

                if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    app.SaveUndoPoint();
                    ApplyPaintBrush(clip, mouse_pos, grid_origin, px_per_tick, note_height);
                }
            }
        }
        else
        {

            if (edge_hovered || m_is_resizing)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            // 1. Double Click Note Creation
            if (canvas_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered_note_idx < 0)
            {
                if (rel_x <= clip_px_width)
                {
                    if (snapped_hover_tick < clip.duration)
                    {
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
            }
            // 2. Empty Space Click (Start Box Selection & Move Playhead to Grid Snap)
            else if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_note_idx < 0 && !io.KeyAlt)
            {
                app.transport.current_tick = clip.start_tick + snapped_hover_tick;

                m_is_box_selecting = true;
                m_box_select_start = mouse_pos;

                if (!io.KeyShift && !io.KeyCtrl)
                {
                    for (auto &note : clip.notes)
                        note.selected = false;
                }
            }

            // 3. Note Select / Drag / Duplicate Handler
            if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_note_idx >= 0)
            {
                m_active_note_idx = hovered_note_idx;
                m_drag_mouse_start = mouse_pos;
                m_selected_initial_states.clear();

                // Update Selection
                if (!io.KeyShift && !io.KeyCtrl && !clip.notes[hovered_note_idx].selected)
                {
                    for (auto &n : clip.notes)
                        n.selected = false;
                    clip.notes[hovered_note_idx].selected = true;
                }
                else if (io.KeyShift || io.KeyCtrl)
                {
                    clip.notes[hovered_note_idx].selected = !clip.notes[hovered_note_idx].selected;
                }

                // Duplicate All Selected Notes on Alt + Drag
                if (io.KeyAlt && !edge_hovered)
                {
                    std::vector<Model::Note> duplicates;
                    for (auto &n : clip.notes)
                    {
                        if (n.selected)
                        {
                            n.selected = false;
                            Model::Note dup = n;
                            dup.selected = true;
                            duplicates.push_back(dup);
                        }
                    }
                    for (const auto &dup : duplicates)
                    {
                        clip.notes.push_back(dup);
                    }
                }

                if (edge_hovered)
                {
                    m_is_resizing = true;
                }
                else
                {
                    m_is_dragging = true;
                }

                // Store initial states for all selected notes to sync group transform
                for (size_t i = 0; i < clip.notes.size(); ++i)
                {
                    if (clip.notes[i].selected)
                    {
                        m_selected_initial_states.push_back({i,
                                                             clip.notes[i].start_tick,
                                                             clip.notes[i].duration,
                                                             clip.notes[i].pitch});
                    }
                }
            }

            // 4. Box Selection Active State, Scrub Playhead & Note Intersection Test
            if (m_is_box_selecting && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                app.transport.current_tick = clip.start_tick + snapped_hover_tick;

                ImVec2 box_min(std::min(m_box_select_start.x, mouse_pos.x), std::min(m_box_select_start.y, mouse_pos.y));
                ImVec2 box_max(std::max(m_box_select_start.x, mouse_pos.x), std::max(m_box_select_start.y, mouse_pos.y));

                draw_list->AddRectFilled(box_min, box_max, IM_COL32(100, 180, 255, 40));
                draw_list->AddRect(box_min, box_max, IM_COL32(120, 200, 255, 200), 0.0f, 0, 1.5f);

                bool append_mode = io.KeyShift || io.KeyCtrl;

                for (auto &note : clip.notes)
                {
                    float nx1 = grid_origin.x + (note.start_tick * px_per_tick);
                    float nx2 = nx1 + (note.duration * px_per_tick);
                    float ny1 = grid_origin.y + (127 - note.pitch) * note_height;
                    float ny2 = ny1 + note_height;

                    bool intersects = (nx1 < box_max.x && nx2 > box_min.x && ny1 < box_max.y && ny2 > box_min.y);
                    if (append_mode)
                    {
                        if (intersects)
                            note.selected = true;
                    }
                    else
                    {
                        note.selected = intersects;
                    }
                }
            }

            // 5. Active Group Drag & Resize
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !m_selected_initial_states.empty())
            {
                float delta_x = mouse_pos.x - m_drag_mouse_start.x;
                float delta_y = mouse_pos.y - m_drag_mouse_start.y;

                int64_t delta_ticks = static_cast<int64_t>(delta_x / px_per_tick);
                int64_t snapped_delta_ticks = (delta_ticks / static_cast<int64_t>(grid_snap_ticks)) * grid_snap_ticks;
                int delta_pitch = -static_cast<int>(std::round(delta_y / note_height));

                if (m_is_resizing)
                {
                    for (const auto &st : m_selected_initial_states)
                    {
                        if (st.index < clip.notes.size())
                        {
                            int64_t new_dur = std::max<int64_t>(grid_snap_ticks, static_cast<int64_t>(st.duration) + snapped_delta_ticks);
                            clip.notes[st.index].duration = static_cast<uint64_t>(new_dur);
                        }
                    }
                }
                else if (m_is_dragging)
                {
                    int64_t min_initial_tick = m_selected_initial_states[0].start_tick;
                    for (const auto &st : m_selected_initial_states)
                    {
                        min_initial_tick = std::min<int64_t>(min_initial_tick, st.start_tick);
                    }
                    int64_t allowed_delta_ticks = std::max(-min_initial_tick, snapped_delta_ticks);

                    for (const auto &st : m_selected_initial_states)
                    {
                        if (st.index < clip.notes.size())
                        {
                            int64_t target_tick = std::max<int64_t>(0, static_cast<int64_t>(st.start_tick) + allowed_delta_ticks);
                            int target_pitch = std::clamp<int>(static_cast<int>(st.pitch) + delta_pitch, 0, 127);

                            clip.notes[st.index].start_tick = static_cast<uint64_t>(target_tick);
                            clip.notes[st.index].pitch = static_cast<uint8_t>(target_pitch);
                        }
                    }
                }
            }

            // 6. Release Drag / Box Selection
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                m_active_note_idx = -1;
                m_is_resizing = false;
                m_is_dragging = false;
                m_is_box_selecting = false;
                m_selected_initial_states.clear();
            }

            // --- Right-Click Context Menu ---
            if (ImGui::BeginPopupContextWindow("NoteManagerContextMenu", ImGuiPopupFlags_MouseButtonRight))
            {
                for (auto *control : m_controls)
                {
                    control->DrawContextMenu(app, clip);
                }
                ImGui::EndPopup();
            }
        }

        // Draw Synced Global Playhead on Top
        DrawPlayhead(app, clip, grid_origin, grid_size, px_per_tick, draw_list);
    }

    void NoteManager::ApplyPaintBrush(Model::Clip& clip, ImVec2 mouse_pos, ImVec2 grid_origin, float px_per_tick, float note_height) {
    for (auto& note : clip.notes) {
        EnsureNoteSegments(note, SEGMENT_TICK_RES);

        float ny1 = grid_origin.y + (127 - note.pitch) * note_height;
        float ny2 = ny1 + note_height;

        if (mouse_pos.y < ny1 - m_brush_radius || mouse_pos.y > ny2 + m_brush_radius) {
            continue;
        }

        bool updated = false;
        size_t seg_count = note.paint_segments.size();

        for (size_t s = 0; s < seg_count; ++s) {
            uint64_t seg_start = note.start_tick + (s * SEGMENT_TICK_RES);
            uint64_t seg_dur = std::min(SEGMENT_TICK_RES, note.duration - (s * SEGMENT_TICK_RES));

            float sx1 = grid_origin.x + (seg_start * px_per_tick);
            float sx2 = sx1 + (seg_dur * px_per_tick);

            ImVec2 seg_center((sx1 + sx2) * 0.5f, (ny1 + ny2) * 0.5f);
            float dx = mouse_pos.x - seg_center.x;
            float dy = mouse_pos.y - seg_center.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= m_brush_radius) {
                float falloff = 1.0f - (dist / m_brush_radius);
                float blend = falloff * 0.2f;

                note.paint_segments[s] = std::clamp(
                    note.paint_segments[s] + (m_brush_strength - note.paint_segments[s]) * blend,
                    0.0f, 1.0f
                );
                updated = true;
            }
        }

        if (updated && m_paint_target == PaintTarget::Velocity) {
            SyncVelocityFromSegments(note);
        }
    }
}

} // namespace gsr::gui