// ClipEditorWindow.cpp
#include "ClipEditorWindow.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace gsr::gui {

ClipEditorWindow::ClipEditorWindow(gsr::App& app) : m_app(app) {}

Model::Clip* ClipEditorWindow::GetSelectedClip() {
    int trk_idx = m_app.view.active_track_index;
    if (trk_idx < 0 || trk_idx >= static_cast<int>(m_app.project.tracks.size())) return nullptr;

    auto& active_track = m_app.project.tracks[trk_idx];
    for (auto& clip : active_track.clips) {
        if (clip.selected) return &clip;
    }

    for (auto& trk : m_app.project.tracks) {
        for (auto& clip : trk.clips) {
            if (clip.selected) return &clip;
        }
    }
    return nullptr;
}

std::string ClipEditorWindow::GetPitchName(uint8_t pitch) {
    static const char* note_names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (pitch / 12) - 1;
    return std::string(note_names[pitch % 12]) + std::to_string(octave);
}

void ClipEditorWindow::DrawPianoKeys(ImDrawList* draw_list, ImVec2 origin, float key_width, float total_height) {
    for (int p = 127; p >= 0; --p) {
        float y1 = origin.y + (127 - p) * m_note_height;
        float y2 = y1 + m_note_height;

        uint8_t note_in_oct = p % 12;
        bool is_black_key = (note_in_oct == 1 || note_in_oct == 3 || note_in_oct == 6 || note_in_oct == 8 || note_in_oct == 10);

        ImU32 bg_color = is_black_key ? IM_COL32(35, 35, 38, 255) : IM_COL32(210, 210, 215, 255);
        ImU32 text_color = is_black_key ? IM_COL32(180, 180, 180, 255) : IM_COL32(20, 20, 20, 255);

        draw_list->AddRectFilled(ImVec2(origin.x, y1), ImVec2(origin.x + key_width, y2), bg_color);
        draw_list->AddLine(ImVec2(origin.x, y2), ImVec2(origin.x + key_width, y2), IM_COL32(80, 80, 80, 100));

        if (note_in_oct == 0 || p == 127 || m_note_height >= 16.0f) {
            std::string label = GetPitchName(static_cast<uint8_t>(p));
            draw_list->AddText(ImVec2(origin.x + 4.0f, y1 + 1.0f), text_color, label.c_str());
        }
    }
}

void ClipEditorWindow::DrawGridBackground(ImDrawList* draw_list, Model::Clip& clip, ImVec2 origin, ImVec2 grid_size) {
    const uint32_t ppq = m_app.project.ppq;
    const float clip_px_width = clip.duration * m_px_per_tick;

    for (int p = 127; p >= 0; --p) {
        float y = origin.y + (127 - p) * m_note_height;
        uint8_t note_in_oct = p % 12;
        bool is_black_key = (note_in_oct == 1 || note_in_oct == 3 || note_in_oct == 6 || note_in_oct == 8 || note_in_oct == 10);

        if (is_black_key) {
            draw_list->AddRectFilled(ImVec2(origin.x, y), ImVec2(origin.x + grid_size.x, y + m_note_height), IM_COL32(0, 0, 0, 25));
        }
        draw_list->AddLine(ImVec2(origin.x, y + m_note_height), ImVec2(origin.x + grid_size.x, y + m_note_height), IM_COL32(255, 255, 255, 12));
    }

    for (uint64_t tick = 0; tick <= clip.duration; tick += m_grid_snap_ticks) {
        float x = origin.x + (tick * m_px_per_tick);
        bool is_bar = (tick % (ppq * 4) == 0);
        bool is_beat = (tick % ppq == 0);

        ImU32 line_col = is_bar ? IM_COL32(200, 200, 200, 100) : (is_beat ? IM_COL32(150, 150, 150, 50) : IM_COL32(100, 100, 100, 25));
        draw_list->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + grid_size.y), line_col);
    }

    if (grid_size.x > clip_px_width) {
        draw_list->AddRectFilled(
            ImVec2(origin.x + clip_px_width, origin.y),
            ImVec2(origin.x + grid_size.x, origin.y + grid_size.y),
            IM_COL32(10, 10, 15, 180)
        );
    }
    draw_list->AddLine(
        ImVec2(origin.x + clip_px_width, origin.y),
        ImVec2(origin.x + clip_px_width, origin.y + grid_size.y),
        IM_COL32(255, 100, 100, 255),
        2.0f
    );
}

void ClipEditorWindow::DrawPlayhead(ImDrawList* draw_list, const Model::Clip& clip, ImVec2 grid_origin, ImVec2 grid_size) {
    int64_t rel_playhead_tick = static_cast<int64_t>(m_app.transport.current_tick) - static_cast<int64_t>(clip.start_tick);
    if (rel_playhead_tick >= 0) {
        float playhead_x = grid_origin.x + (rel_playhead_tick * m_px_per_tick);
        if (playhead_x >= grid_origin.x && playhead_x <= grid_origin.x + grid_size.x) {
            draw_list->AddLine(
                ImVec2(playhead_x, grid_origin.y),
                ImVec2(playhead_x, grid_origin.y + grid_size.y),
                IM_COL32(255, 75, 75, 255),
                2.0f
            );
        }
    }
}

void ClipEditorWindow::DrawSelf() {
    Model::Clip* clip = GetSelectedClip();

    if (!clip) {
        ImGui::TextDisabled("No clip selected. Click a clip in the timeline view to edit.");
        return;
    }

    ImGui::Text("Editing Clip: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", clip->name.c_str());

    ImGui::SameLine(0.0f, 20.0f);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("Zoom H", &m_px_per_tick, 0.005f, 0.1f, "%.3f");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("Zoom V", &m_note_height, 8.0f, 28.0f, "%.0f px");

    ImGui::SameLine();
    uint32_t ppq = m_app.project.ppq;
    int snap_idx = (m_grid_snap_ticks == ppq / 2) ? 0 : ((m_grid_snap_ticks == ppq / 4) ? 1 : 2);
    const char* snap_options[] = { "1/8 Note", "1/16 Note", "1/32 Note" };
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("Snap", &snap_idx, snap_options, IM_ARRAYSIZE(snap_options))) {
        if (snap_idx == 0) m_grid_snap_ticks = ppq / 2;
        else if (snap_idx == 1) m_grid_snap_ticks = ppq / 4;
        else if (snap_idx == 2) m_grid_snap_ticks = ppq / 8;
    }

    // Edit Mode Combo
    ImGui::SameLine();
    int mode_idx = static_cast<int>(m_edit_mode);
    const char* modes[] = { "Select Mode", "Paint Mode" };
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("Tool", &mode_idx, modes, IM_ARRAYSIZE(modes))) {
        m_edit_mode = static_cast<PianoRollEditMode>(mode_idx);
    }

    if (m_edit_mode == PianoRollEditMode::Paint) {
        ImGui::SameLine();
        int target_idx = static_cast<int>(m_paint_interaction.target);
        const char* targets[] = { "Velocity", "Aftertouch" };
        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::Combo("Target", &target_idx, targets, IM_ARRAYSIZE(targets))) {
            m_paint_interaction.target = static_cast<PaintTarget>(target_idx);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::SliderFloat("Radius", &m_paint_interaction.brush_radius, 5.0f, 100.0f, "%.0f px");

        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::SliderFloat("Value", &m_paint_interaction.brush_strength, 0.0f, 1.0f, "%.2f");
    }

    ImGui::Separator();

    constexpr float KEY_WIDTH = 55.0f;
    const float total_grid_height = 128.0f * m_note_height;
    float pending_vertical_scroll = -1.0f;

    ImGui::BeginChild("PianoRollScrollArea", ImVec2(0, 0), true);

    ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
    const float outer_width = ImGui::GetContentRegionAvail().x;
    const float outer_scroll_y = ImGui::GetScrollY();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;

    DrawPianoKeys(draw_list, canvas_origin, KEY_WIDTH, total_grid_height);

    ImGui::SetCursorScreenPos(ImVec2(canvas_origin.x + KEY_WIDTH, canvas_origin.y));
    ImGui::BeginChild(
        "PianoRollGrid",
        ImVec2(std::max(outer_width - KEY_WIDTH, 50.0f), total_grid_height),
        true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    ImVec2 grid_origin = ImGui::GetCursorScreenPos();
    const float total_grid_width = std::max(
        ImGui::GetContentRegionAvail().x,
        clip->duration * m_px_per_tick + 200.0f
    );
    ImVec2 grid_size(total_grid_width, total_grid_height);

    ImGui::SetCursorScreenPos(grid_origin);
    ImGui::InvisibleButton("PianoRollCanvas", grid_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool canvas_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    // Zoom & Pan Wheel Controls
    if (canvas_hovered) {
        if (io.KeyShift && io.MouseWheel != 0.0f) {
            const float old_px_per_tick = m_px_per_tick;
            const float mouse_tick = (mouse_pos.x - grid_origin.x + ImGui::GetScrollX()) / old_px_per_tick;
            float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
            m_px_per_tick = std::clamp(m_px_per_tick * zoom_factor, 0.005f, 0.2f);
            ImGui::SetScrollX(mouse_tick * m_px_per_tick - (mouse_pos.x - grid_origin.x));
        }
        if (io.KeyAlt && io.MouseWheel != 0.0f) {
            const float old_note_height = m_note_height;
            const float mouse_pitch = (mouse_pos.y - grid_origin.y + outer_scroll_y) / old_note_height;
            float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
            m_note_height = std::clamp(m_note_height * zoom_factor, 8.0f, 28.0f);
            pending_vertical_scroll = mouse_pitch * m_note_height - (mouse_pos.y - grid_origin.y);
        }
        if (!io.KeyShift && !io.KeyAlt && io.MouseWheel != 0.0f) {
            pending_vertical_scroll = outer_scroll_y - io.MouseWheel * ImGui::GetFontSize() * 3.0f;
        }
        if (io.KeyAlt && io.MouseDelta.x != 0.0f) {
            ImGui::SetScrollX(ImGui::GetScrollX() - io.MouseDelta.x);
        }
        io.MouseWheel = 0.0f;
        io.MouseWheelH = 0.0f;
    }

    DrawGridBackground(draw_list, *clip, grid_origin, grid_size);

    // Draw Notes with Segment-based Transparency
    int hovered_note_idx = -1;
    bool edge_hovered = false;

    for (size_t i = 0; i < clip->notes.size(); ++i) {
        auto& note = clip->notes[i];
        NotePaintInteraction::EnsureNoteSegments(note, NotePaintInteraction::SEGMENT_TICK_RES);

        float nx1 = grid_origin.x + (note.start_tick * m_px_per_tick);
        float nx2 = nx1 + (note.duration * m_px_per_tick);
        float ny1 = grid_origin.y + (127 - note.pitch) * m_note_height;
        float ny2 = ny1 + m_note_height;

        ImVec2 n_min(nx1, ny1 + 1.0f);
        ImVec2 n_max(nx2, ny2 - 1.0f);

        float val_norm = note.paint_segments[0];
        int alpha = static_cast<int>(std::clamp(val_norm * 230.0f + 25.0f, 25.0f, 255.0f));

        ImU32 fill_col = note.selected ? IM_COL32(255, 210, 80, alpha) : IM_COL32(230, 150, 40, alpha);
        ImU32 border_col = note.selected ? IM_COL32(255, 255, 200, 255) : IM_COL32(255, 190, 100, alpha);

        draw_list->AddRectFilled(n_min, n_max, fill_col, 2.0f);
        draw_list->AddRect(n_min, n_max, border_col, 2.0f);

        if (mouse_pos.x >= n_min.x && mouse_pos.x <= n_max.x && mouse_pos.y >= n_min.y && mouse_pos.y <= n_max.y) {
            hovered_note_idx = static_cast<int>(i);
            if (mouse_pos.x >= n_max.x - 6.0f) edge_hovered = true;
        }
    }

    // Dispatch Interactions to Active Mode Class
    if (m_edit_mode == PianoRollEditMode::Paint) {
        m_paint_interaction.ProcessPaint(
            m_app, *clip, mouse_pos, grid_origin, m_px_per_tick, m_note_height, canvas_hovered, draw_list
        );
    } else {
        m_select_interaction.ProcessSelect(
            m_app, *clip, mouse_pos, grid_origin, m_px_per_tick, m_note_height, m_grid_snap_ticks, canvas_hovered, hovered_note_idx, edge_hovered
        );
    }

    DrawPlayhead(draw_list, *clip, grid_origin, grid_size);

    ImGui::EndChild();
    if (pending_vertical_scroll >= 0.0f) {
        ImGui::SetScrollY(pending_vertical_scroll);
    }
    ImGui::EndChild();
}

} // namespace gsr::gui