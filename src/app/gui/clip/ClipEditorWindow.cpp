#include "ClipEditorWindow.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace gsr::gui {

ClipEditorWindow::ClipEditorWindow(gsr::App& app)
    : m_app(app) {}

Model::Clip* ClipEditorWindow::GetSelectedClip() {
    int trk_idx = m_app.view.active_track_index;
    if (trk_idx < 0 || trk_idx >= static_cast<int>(m_app.project.tracks.size())) {
        return nullptr;
    }

    auto& active_track = m_app.project.tracks[trk_idx];
    for (auto& clip : active_track.clips) {
        if (clip.selected) return &clip;
    }

    for (auto& trk : m_app.project.tracks) {
        for (auto& clip : trk.clips) {
            if (clip.selected) return &clip;
        }
    }

    const auto& sel = m_app.view.cell_selection;
    if (sel.active && sel.track_index == trk_idx) {
        uint32_t ticks_per_bar = m_app.project.ppq * 4;
        uint64_t sel_start = sel.start_bar * ticks_per_bar;
        uint64_t sel_dur = sel.num_bars * ticks_per_bar;

        for (auto& clip : active_track.clips) {
            if ((sel_start < clip.start_tick + clip.duration) && (sel_start + sel_dur > clip.start_tick)) {
                return &clip;
            }
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

    ImGui::Separator();

    constexpr float KEY_WIDTH = 55.0f;
    const float total_grid_height = 128.0f * m_note_height;
    const float total_grid_width = std::max(ImGui::GetContentRegionAvail().x - KEY_WIDTH, clip->duration * m_px_per_tick + 200.0f);

    ImGui::BeginChild("PianoRollScrollArea", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    DrawPianoKeys(draw_list, canvas_origin, KEY_WIDTH, total_grid_height);

    ImVec2 grid_origin(canvas_origin.x + KEY_WIDTH, canvas_origin.y);
    ImVec2 grid_size(total_grid_width, total_grid_height);

    // Draw canvas widget and capture hover state even when active/clicked
    ImGui::SetCursorScreenPos(grid_origin);
    ImGui::InvisibleButton("PianoRollCanvas", grid_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool canvas_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    DrawGridBackground(draw_list, *clip, grid_origin, grid_size);

    // Pass canvas_hovered into NoteManager
    m_note_manager.ProcessAndDrawNotes(
        m_app,
        *clip,
        grid_origin,
        grid_size,
        m_px_per_tick,
        m_note_height,
        m_grid_snap_ticks,
        canvas_hovered
    );

    ImGui::EndChild();
}

} // namespace gsr::gui