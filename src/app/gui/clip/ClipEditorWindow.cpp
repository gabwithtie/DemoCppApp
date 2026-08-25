#include "ClipEditorWindow.hpp"
#include "gui/timeline/util/TimelineWarp.hpp" // Add this include
#include <algorithm>
#include <cmath>
#include <string>

namespace gsr::gui
{
    //Returns the other key that was pressed other than shift.
    ImGuiKey IsOnlyShiftDown()
    {
        ImGuiIO& io = ImGui::GetIO();

        // 1. Shift must be active, and no other modifier flag (Ctrl, Alt, Super) can be set
        if (io.KeyMods != ImGuiMod_Shift)
            return ImGuiKey::ImGuiKey_None;

        // 2. Scan standard keys, skipping modifier aliases, mouse, and gamepad inputs
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k)
        {
            ImGuiKey key = static_cast<ImGuiKey>(k);

            // Skip physical modifier keys (Left/Right Shift, Ctrl, Alt, Super)
            if (key >= ImGuiKey_LeftCtrl && key <= ImGuiKey_RightSuper) continue;

            // Skip ImGui virtual modifier alias keys (ModCtrl, ModShift, ModAlt, ModSuper)
            if (key >= ImGuiKey_ReservedForModCtrl && key <= ImGuiKey_ReservedForModSuper) continue;

            // Skip mouse buttons and gamepad inputs
            if (key >= ImGuiKey_MouseLeft && key <= ImGuiKey_GamepadRStickRight) continue;

            if (ImGui::IsKeyDown(key))
                return key;
        }

        return ImGuiKey::ImGuiKey_None;
    }

    ImU32 GetNotePaintColor(float val_127, bool is_selected)
    {
        float r = 0.0f, g = 0.0f, b = 0.0f;
        float alpha = 255.0f;

        if (val_127 <= 70.0f)
        {
            // Pure Blue below 70: Alpha scales down linearly as value approaches 0
            r = 0.0f; g = 0.0f; b = 255.0f;
            float norm = std::clamp(val_127 / 70.0f, 0.0f, 1.0f);
            alpha = norm * 200.0f + 55.0f; // Scale alpha between 55 and 255
        }
        else if (val_127 < 90.0f)
        {
            // Transition: Pure Blue (70) -> Pure Green (90)
            float t = (val_127 - 70.0f) / 20.0f;
            r = 0.0f;
            g = t * 255.0f;
            b = (1.0f - t) * 255.0f;
        }
        else if (val_127 <= 110.0f)
        {
            // 100% Green between 90 and 110
            r = 0.0f; g = 255.0f; b = 0.0f;
        }
        else if (val_127 < 120.0f)
        {
            // Transition: Pure Green (110) -> Pure Red (120)
            float t = (val_127 - 110.0f) / 10.0f;
            r = t * 255.0f;
            g = (1.0f - t) * 255.0f;
            b = 0.0f;
        }
        else
        {
            // 100% Red at 120+
            r = 255.0f; g = 0.0f; b = 0.0f;
        }

        // Selection boost
        if (is_selected)
        {
            r = std::min(255.0f, r + 70.0f);
            g = std::min(255.0f, g + 70.0f);
            b = std::min(255.0f, b + 70.0f);
        }

        return IM_COL32(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(alpha));
    }

    ClipEditorWindow::ClipEditorWindow(gsr::App &app) : m_app(app) {}

    Model::Clip *ClipEditorWindow::GetSelectedClip()
    {
        int trk_idx = m_app.view.active_track_index;
        if (trk_idx < 0 || trk_idx >= static_cast<int>(m_app.project.tracks.size()))
            return nullptr;

        auto &active_track = m_app.project.tracks[trk_idx];
        for (auto &clip : active_track.clips)
        {
            if (clip.selected)
                return &clip;
        }

        for (auto &trk : m_app.project.tracks)
        {
            for (auto &clip : trk.clips)
            {
                if (clip.selected)
                    return &clip;
            }
        }
        return nullptr;
    }

    std::string ClipEditorWindow::GetPitchName(uint8_t pitch)
    {
        static const char *note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (pitch / 12) - 1;
        return std::string(note_names[pitch % 12]) + std::to_string(octave);
    }

    void ClipEditorWindow::DrawPianoKeys(ImDrawList *draw_list, ImVec2 origin, float key_width, float total_height)
    {
        for (int p = 127; p >= 0; --p)
        {
            float y1 = origin.y + (127 - p) * m_note_height;
            float y2 = y1 + m_note_height;

            uint8_t note_in_oct = p % 12;
            bool is_black_key = (note_in_oct == 1 || note_in_oct == 3 || note_in_oct == 6 || note_in_oct == 8 || note_in_oct == 10);

            ImU32 bg_color = is_black_key ? IM_COL32(35, 35, 38, 255) : IM_COL32(210, 210, 215, 255);
            ImU32 text_color = is_black_key ? IM_COL32(180, 180, 180, 255) : IM_COL32(20, 20, 20, 255);

            draw_list->AddRectFilled(ImVec2(origin.x, y1), ImVec2(origin.x + key_width, y2), bg_color);
            draw_list->AddLine(ImVec2(origin.x, y2), ImVec2(origin.x + key_width, y2), IM_COL32(80, 80, 80, 100));

            if (note_in_oct == 0 || p == 127 || m_note_height >= 16.0f)
            {
                std::string label = GetPitchName(static_cast<uint8_t>(p));
                draw_list->AddText(ImVec2(origin.x + 4.0f, y1 + 1.0f), text_color, label.c_str());
            }
        }
    }

    void ClipEditorWindow::DrawGridBackground(ImDrawList *draw_list, Model::Clip &clip, ImVec2 origin, ImVec2 grid_size)
    {
        const uint32_t ppq = m_app.project.ppq;
        const float clip_px_width = clip.duration * m_px_per_tick;

        for (int p = 127; p >= 0; --p)
        {
            float y = origin.y + (127 - p) * m_note_height;
            uint8_t note_in_oct = p % 12;
            bool is_black_key = (note_in_oct == 1 || note_in_oct == 3 || note_in_oct == 6 || note_in_oct == 8 || note_in_oct == 10);

            if (is_black_key)
            {
                draw_list->AddRectFilled(ImVec2(origin.x, y), ImVec2(origin.x + grid_size.x, y + m_note_height), IM_COL32(0, 0, 0, 25));
            }
            draw_list->AddLine(ImVec2(origin.x, y + m_note_height), ImVec2(origin.x + grid_size.x, y + m_note_height), IM_COL32(255, 255, 255, 12));
        }

        for (uint64_t tick = 0; tick <= clip.duration; tick += m_grid_snap_ticks)
        {
            float x = origin.x + (tick * m_px_per_tick);
            bool is_bar = (tick % (ppq * 4) == 0);
            bool is_beat = (tick % ppq == 0);

            ImU32 line_col = is_bar ? IM_COL32(200, 200, 200, 100) : (is_beat ? IM_COL32(150, 150, 150, 50) : IM_COL32(100, 100, 100, 25));
            draw_list->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + grid_size.y), line_col);
        }

        if (grid_size.x > clip_px_width)
        {
            draw_list->AddRectFilled(
                ImVec2(origin.x + clip_px_width, origin.y),
                ImVec2(origin.x + grid_size.x, origin.y + grid_size.y),
                IM_COL32(10, 10, 15, 180));
        }
        draw_list->AddLine(
            ImVec2(origin.x + clip_px_width, origin.y),
            ImVec2(origin.x + clip_px_width, origin.y + grid_size.y),
            IM_COL32(255, 100, 100, 255),
            2.0f);
    }

    void ClipEditorWindow::DrawPlayhead(ImDrawList *draw_list, const Model::Clip &clip, ImVec2 grid_origin, ImVec2 grid_size, bool is_focused)
    {
        // 1. Draw Internal Edit Playhead (Cyan) - ONLY WHEN FOCUSED
        if (is_focused)
        {
            float int_playhead_x = grid_origin.x + (m_internal_playhead_tick * m_px_per_tick);
            if (int_playhead_x >= grid_origin.x && int_playhead_x <= grid_origin.x + grid_size.x)
            {
                draw_list->AddLine(
                    ImVec2(int_playhead_x, grid_origin.y),
                    ImVec2(int_playhead_x, grid_origin.y + grid_size.y),
                    IM_COL32(80, 200, 255, 255),
                    2.0f);
            }
        }

        // 2. Draw Main Software Playhead (Red)
        double current_source_tick = TargetToSourceTick(m_app.project, static_cast<double>(m_app.transport.current_tick));
        int64_t rel_playhead_tick = static_cast<int64_t>(current_source_tick) - static_cast<int64_t>(clip.start_tick);
        if (rel_playhead_tick >= 0)
        {
            float playhead_x = grid_origin.x + (rel_playhead_tick * m_px_per_tick);
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

    void ClipEditorWindow::DrawSelf()
    {
        Model::Clip *clip = GetSelectedClip();

        if (!clip)
        {
            ImGui::TextDisabled("No clip selected. Click a clip in the timeline view to edit.");
            return;
        }

        // Check if the current window or any of its children are focused
        bool is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        // Sync Software Playhead strictly when entering Play state AND focused
        bool is_playing = m_app.transport.state == PlaybackState::Playing;
        if (is_playing && !m_was_playing && is_focused)
        {
            uint64_t source_tick = clip->start_tick + m_internal_playhead_tick;
            m_app.transport.current_tick = static_cast<uint64_t>(std::max(
                0.0, SourceToTargetTick(m_app.project, static_cast<double>(source_tick))));
        }
        m_was_playing = is_playing;

        ImGui::Text("Editing Clip: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", clip->name.c_str());

        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Zoom H", &m_px_per_tick, 0.005f, 0.1f, "%.3f");

        // DrawSelf() inside ClipEditorWindow.cpp
        ImGui::SameLine();
        uint32_t ppq = m_app.project.ppq;

        int snap_idx = -1;
        const char* snap_labels[G_SNAP_OPTION_COUNT];
        for (int i = 0; i < G_SNAP_OPTION_COUNT; ++i)
        {
            snap_labels[i] = G_SNAP_OPTIONS[i].label;
            if (m_grid_snap_ticks == GetTicksForSnap(G_SNAP_OPTIONS[i].resolution, ppq))
            {
                snap_idx = i;
            }
        }

        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::Combo("Snap", &snap_idx, snap_labels, G_SNAP_OPTION_COUNT))
        {
            if (snap_idx >= 0 && snap_idx < G_SNAP_OPTION_COUNT)
            {
                m_grid_snap_ticks = GetTicksForSnap(G_SNAP_OPTIONS[snap_idx].resolution, ppq);
            }
        }

        // Edit Mode Combo
        ImGui::SameLine();
        int mode_idx = static_cast<int>(m_edit_mode);
        const char *modes[] = {"Select Mode", "Paint Mode"};
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::Combo("Tool", &mode_idx, modes, IM_ARRAYSIZE(modes)))
        {
            m_edit_mode = static_cast<PianoRollEditMode>(mode_idx);
        }

        if (m_edit_mode == PianoRollEditMode::Paint)
        {
            ImGui::SameLine();
            int target_idx = static_cast<int>(m_paint_interaction.target);
            const char *targets[] = {"Velocity", "Aftertouch"};
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::Combo("Target", &target_idx, targets, IM_ARRAYSIZE(targets)))
            {
                m_paint_interaction.target = static_cast<PaintTarget>(target_idx);
            }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::SliderFloat("Radius", &m_paint_interaction.brush_radius, 5.0f, 200.0f, "%.0f px");

        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::SliderFloat("Strength", &m_paint_interaction.brush_strength, 0.001f, 0.100f, "%.3f");        }

        ImGui::Separator();

        constexpr float KEY_WIDTH = 55.0f;
        float pending_vertical_scroll = -1.0f;
        float pending_horizontal_scroll = -1.0f;

        ImGui::BeginChild("PianoRollScrollArea", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
        const float outer_width = ImGui::GetContentRegionAvail().x;
        const float outer_height = ImGui::GetContentRegionAvail().y;
        const float outer_scroll_y = ImGui::GetScrollY();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 mouse_pos = io.MousePos;

        const float grid_viewport_w = std::max(outer_width - KEY_WIDTH, 50.0f);
        bool scroll_area_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        // Process Vertical Zoom & Pan BEFORE defining canvas heights
        if (scroll_area_hovered || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            if (io.KeyAlt && io.MouseWheel != 0.0f)
            {
                const float old_note_height = m_note_height;
                const float half_vh = outer_height * 0.5f;
                const float center_pitch_unit = (outer_scroll_y + half_vh) / old_note_height;

                float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
                m_note_height = std::clamp(m_note_height * zoom_factor, 8.0f, 28.0f);

                pending_vertical_scroll = std::max(0.0f, center_pitch_unit * m_note_height - half_vh);
            }
            if (!io.KeyShift && !io.KeyAlt && io.MouseWheel != 0.0f)
            {
                pending_vertical_scroll = std::max(0.0f, outer_scroll_y - io.MouseWheel * ImGui::GetFontSize() * 3.0f);
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.MouseDelta.y != 0.0f)
            {
                const float base_y = (pending_vertical_scroll >= 0.0f) ? pending_vertical_scroll : outer_scroll_y;
                pending_vertical_scroll = std::max(0.0f, base_y - io.MouseDelta.y);
            }
        }

        const float total_grid_height = 128.0f * m_note_height;

        DrawPianoKeys(draw_list, canvas_origin, KEY_WIDTH, total_grid_height);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetCursorScreenPos(ImVec2(canvas_origin.x + KEY_WIDTH, canvas_origin.y));

        ImGui::BeginChild(
            "PianoRollGrid",
            ImVec2(grid_viewport_w, total_grid_height + ImGui::GetStyle().ScrollbarSize),
            false,
            ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        );
        ImGui::PopStyleVar();

        //===== INPUT HANDLING =====//

        // Process Horizontal Zoom & Pan
        if (scroll_area_hovered || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            auto other_key = IsOnlyShiftDown();
            auto other_key_is_scroll = other_key == ImGuiKey::ImGuiKey_MouseWheelY;

            if (other_key_is_scroll && io.MouseWheel != 0.0f)
            {
                const float old_px_per_tick = m_px_per_tick;
                const float half_vw = grid_viewport_w * 0.5f;
                const float current_scroll_x = ImGui::GetScrollX();
                const float center_tick = (current_scroll_x + half_vw) / old_px_per_tick;

                float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
                m_px_per_tick = std::clamp(m_px_per_tick * zoom_factor, 0.005f, 0.2f);

                pending_horizontal_scroll = std::max(0.0f, center_tick * m_px_per_tick - half_vw);
                io.MouseWheel = 0.0f;
                io.MouseWheelH = 0.0f;
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.MouseDelta.x != 0.0f)
            {
                const float base_x = (pending_horizontal_scroll >= 0.0f) ? pending_horizontal_scroll : ImGui::GetScrollX();
                pending_horizontal_scroll = std::max(0.0f, base_x - io.MouseDelta.x);
            }
        }

        ImVec2 grid_origin = ImGui::GetCursorScreenPos();
        const float total_grid_width = std::max(
            grid_viewport_w,
            clip->duration * m_px_per_tick + 200.0f
        );
        ImVec2 grid_size(total_grid_width, total_grid_height);

        ImGui::SetCursorScreenPos(grid_origin);
        ImGui::InvisibleButton("PianoRollCanvas", grid_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        bool canvas_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        // Position Internal Playhead (Alt + Left Click/Drag on canvas)
        if (canvas_hovered && io.KeyAlt && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            float rel_x = mouse_pos.x - grid_origin.x;
            if (rel_x >= 0.0f)
            {
                uint64_t target_tick = static_cast<uint64_t>(rel_x / m_px_per_tick);
                if (m_grid_snap_ticks > 0)
                {
                    target_tick = (target_tick / m_grid_snap_ticks) * m_grid_snap_ticks;
                }
                m_internal_playhead_tick = std::min(target_tick, clip->duration);
            }
        }

        if (pending_horizontal_scroll >= 0.0f)
        {
            ImGui::SetScrollX(pending_horizontal_scroll);
        }

        DrawGridBackground(draw_list, *clip, grid_origin, grid_size);

        // Draw Notes
        int hovered_note_idx = -1;
        bool edge_hovered = false;

        for (size_t i = 0; i < clip->notes.size(); ++i)
        {
            auto &note = clip->notes[i];
            NotePaintInteraction::EnsureNoteSegments(note, NotePaintInteraction::SEGMENT_TICK_RES);

            float nx1 = grid_origin.x + (note.start_tick * m_px_per_tick);
            float nx2 = nx1 + (note.duration * m_px_per_tick);
            float ny1 = grid_origin.y + (127 - note.pitch) * m_note_height;
            float ny2 = ny1 + m_note_height;

            ImVec2 n_min(nx1, ny1 + 1.0f);
            ImVec2 n_max(nx2, ny2 - 1.0f);

            float note_w = n_max.x - n_min.x;
            float note_h = n_max.y - n_min.y;

            // Retrieve target value scaled to 0-127
            float val_norm = note.paint_segments.empty() ? 0.0f : note.paint_segments[0];
            float val_127 = std::clamp(val_norm * 127.0f, 0.0f, 127.0f);

            ImU32 fill_col = GetNotePaintColor(val_127, note.selected);
            ImU32 border_col = note.selected ? IM_COL32(255, 255, 200, 255) : IM_COL32(255, 255, 255, 100);

            draw_list->AddRectFilled(n_min, n_max, fill_col, 2.0f);
            draw_list->AddRect(n_min, n_max, border_col, 2.0f);

            // Display target value text if the note box is large enough at current zoom level
            std::string val_str = std::to_string(static_cast<int>(std::round(val_127)));
            ImVec2 text_size = ImGui::CalcTextSize(val_str.c_str());

            // Requires 4px horizontal and 2px vertical padding minimum
            if (note_w >= text_size.x + 4.0f && note_h >= text_size.y + 2.0f)
            {
                ImVec2 text_pos(
                    n_min.x + (note_w - text_size.x) * 0.5f,
                    n_min.y + (note_h - text_size.y) * 0.5f
                );
                draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 230), val_str.c_str());
            }

            if (mouse_pos.x >= n_min.x && mouse_pos.x <= n_max.x && mouse_pos.y >= n_min.y && mouse_pos.y <= n_max.y)
            {
                hovered_note_idx = static_cast<int>(i);
                if (mouse_pos.x >= n_max.x - 6.0f)
                    edge_hovered = true;
            }
        }

        // Process interactions
        if (m_edit_mode == PianoRollEditMode::Paint)
        {
            m_paint_interaction.ProcessPaint(
                m_app, *clip, mouse_pos, grid_origin, m_px_per_tick, m_note_height, canvas_hovered, draw_list);
        }
        else
        {
            m_select_interaction.ProcessSelect(
                m_app, *clip, m_internal_playhead_tick, mouse_pos, grid_origin, m_px_per_tick, m_note_height, m_grid_snap_ticks, canvas_hovered, hovered_note_idx, edge_hovered);
        }

        DrawPlayhead(draw_list, *clip, grid_origin, grid_size, is_focused);

        ImGui::EndChild();
        if (pending_vertical_scroll >= 0.0f)
        {
            ImGui::SetScrollY(pending_vertical_scroll);
        }
        ImGui::EndChild();
    }

} // namespace gsr::gui