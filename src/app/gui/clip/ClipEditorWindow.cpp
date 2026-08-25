#include "ClipEditorWindow.hpp"
#include "gui/timeline/util/TimelineWarp.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace gsr::gui
{
    ImU32 GetNotePaintColor(float val_127, bool is_selected)
    {
        // Color Stop Table: { Value, ImVec4(R, G, B, Alpha) }
        static const struct ColorStop { float val; ImVec4 col; } stops[] = {
            {   0.0f, {   0.0f,   0.0f, 255.0f,  55.0f } }, // Faded Blue
            {  10.0f, {   0.0f,   0.0f, 255.0f, 255.0f } }, // Solid Blue
            {  70.0f, {   0.0f, 255.0f,   0.0f, 255.0f } }, // Solid Green
            {  80.0f, {   0.0f, 255.0f,   0.0f, 255.0f } }, // Solid Green
            { 130.0f, { 255.0f,   0.0f,   0.0f, 255.0f } }  // Solid Red
        };

        auto lerp = [](const ImVec4& a, const ImVec4& b, float t) {
            return ImVec4(
                a.x + t * (b.x - a.x),
                a.y + t * (b.y - a.y),
                a.z + t * (b.z - a.z),
                a.w + t * (b.w - a.w)
            );
        };

        val_127 = std::clamp(val_127, 0.0f, 120.0f);

        // Find active range and lerp
        ImVec4 col = stops[4].col;
        for (size_t i = 0; i < 4; ++i)
        {
            if (val_127 <= stops[i + 1].val)
            {
                float t = (val_127 - stops[i].val) / (stops[i + 1].val - stops[i].val);
                col = lerp(stops[i].col, stops[i + 1].col, t);
                break;
            }
        }

        // Selection highlight boost
        if (is_selected)
        {
            col.x = std::min(255.0f, col.x + 70.0f);
            col.y = std::min(255.0f, col.y + 70.0f);
            col.z = std::min(255.0f, col.z + 70.0f);
        }

        return IM_COL32(static_cast<int>(col.x), static_cast<int>(col.y), static_cast<int>(col.z), static_cast<int>(col.w));
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

        bool is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

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

        ImGui::SameLine();
        uint32_t ppq = m_app.project.ppq;

        int snap_idx = -1;
        const char *snap_labels[G_SNAP_OPTION_COUNT];
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
            ImGui::SliderFloat("Strength", &m_paint_interaction.brush_strength, 0.001f, 0.100f, "%.3f");
        }

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

        // Process Vertical Input
        m_input_handler.ProcessVerticalScrollAndZoom(
            scroll_area_hovered, outer_height, outer_scroll_y, m_note_height, pending_vertical_scroll);

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

        // Process Horizontal Input
        m_input_handler.ProcessHorizontalScrollAndZoom(
            scroll_area_hovered, grid_viewport_w, m_px_per_tick, pending_horizontal_scroll);

        ImVec2 grid_origin = ImGui::GetCursorScreenPos();
        const float total_grid_width = std::max(
            grid_viewport_w,
            clip->duration * m_px_per_tick + 200.0f
        );
        ImVec2 grid_size(total_grid_width, total_grid_height);

        ImGui::SetCursorScreenPos(grid_origin);
        ImGui::InvisibleButton("PianoRollCanvas", grid_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        bool canvas_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        // Process Playhead Input
        m_input_handler.ProcessPlayheadPositioning(
            canvas_hovered, mouse_pos, grid_origin, m_px_per_tick, m_grid_snap_ticks, clip->duration, m_internal_playhead_tick);

        if (pending_horizontal_scroll >= 0.0f)
        {
            ImGui::SetScrollX(pending_horizontal_scroll);
        }

        DrawGridBackground(draw_list, *clip, grid_origin, grid_size);

        // Draw Notes & Calculate Hovering
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

            float val_norm = note.paint_segments.empty() ? 0.0f : note.paint_segments[0];
            float val_127 = std::clamp(val_norm * 127.0f, 0.0f, 127.0f);

            ImU32 fill_col = GetNotePaintColor(val_127, note.selected);
            ImU32 border_col = note.selected ? IM_COL32(255, 255, 200, 255) : IM_COL32(255, 255, 255, 100);

            draw_list->AddRectFilled(n_min, n_max, fill_col, 2.0f);
            draw_list->AddRect(n_min, n_max, border_col, 2.0f);

            std::string val_str = std::to_string(static_cast<int>(std::round(val_127)));
            ImVec2 text_size = ImGui::CalcTextSize(val_str.c_str());

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

        // Process Note Interactions (Paint/Select)
        m_input_handler.ProcessNoteInteractions(
            m_app, *clip, m_edit_mode, m_paint_interaction, m_select_interaction,
            m_internal_playhead_tick, mouse_pos, grid_origin, m_px_per_tick, m_note_height,
            m_grid_snap_ticks, canvas_hovered, hovered_note_idx, edge_hovered, draw_list);

        DrawPlayhead(draw_list, *clip, grid_origin, grid_size, is_focused);

        ImGui::EndChild();
        if (pending_vertical_scroll >= 0.0f)
        {
            ImGui::SetScrollY(pending_vertical_scroll);
        }
        ImGui::EndChild();
    }

} // namespace gsr::gui