#include "ClipManager.hpp"
#include <algorithm>
#include <string>

namespace gsr::gui {

bool ClipManager::ClipsOverlap(uint64_t start1, uint64_t dur1, uint64_t start2, uint64_t dur2) {
    return (start1 < start2 + dur2) && (start1 + dur1 > start2);
}

void ClipManager::DrawRuler(gsr::App& app, float ruler_height) {
    const float px_per_tick = app.view.px_per_tick;
    const uint64_t scroll_tick = app.view.scroll_tick;
    const uint32_t ppq = app.project.ppq;
    const uint32_t ticks_per_bar = ppq * 4;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(std::max(ImGui::GetContentRegionAvail().x, 50.0f), ruler_height);

    ImGui::InvisibleButton("RulerCanvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft);

    ImGuiIO& io = ImGui::GetIO();
    const bool is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    // Zoom & Horizontal Pan Controls
    if (is_hovered) {
        if (io.KeyShift && io.MouseWheel != 0.0f) {
            float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
            app.view.px_per_tick = std::clamp(app.view.px_per_tick * zoom_factor, 0.005f, 0.2f);
            io.MouseWheel = 0.0f;
            io.MouseWheelH = 0.0f;
        }
        if (io.KeyAlt && io.MouseDelta.x != 0.0f) {
            int64_t delta_ticks = static_cast<int64_t>(io.MouseDelta.x / app.view.px_per_tick);
            int64_t new_scroll = static_cast<int64_t>(app.view.scroll_tick) - delta_ticks;
            app.view.scroll_tick = static_cast<uint64_t>(std::max<int64_t>(0, new_scroll));
        }
    }

    // Playhead Scrubbing on Ruler
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !io.KeyAlt) {
        float rel_mouse_x = io.MousePos.x - canvas_pos.x;
        int64_t target_tick = static_cast<int64_t>(rel_mouse_x / app.view.px_per_tick) + app.view.scroll_tick;
        app.transport.current_tick = std::max<int64_t>(0, target_tick);
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(32, 34, 38, 255));
    draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y - 1.0f), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y - 1.0f), IM_COL32(80, 80, 80, 255));

    const double source_scroll_tick = std::max(0.0, TargetToSourceTick(app.project, static_cast<double>(app.view.scroll_tick)));
    const double source_view_end = TargetToSourceTick(
        app.project,
        static_cast<double>(app.view.scroll_tick) + canvas_size.x / app.view.px_per_tick);
    int start_bar = static_cast<int>(source_scroll_tick / ticks_per_bar);
    int end_bar = static_cast<int>(source_view_end / ticks_per_bar) + 2;

    for (int b = start_bar; b <= end_bar; ++b) {
        const double source_bar_tick = static_cast<double>(b) * ticks_per_bar;
        const double target_bar_tick = SourceToTargetTick(app.project, source_bar_tick);
        float line_x = canvas_pos.x + static_cast<float>(target_bar_tick - app.view.scroll_tick) * app.view.px_per_tick;

        if (line_x >= canvas_pos.x - 20.0f && line_x <= canvas_pos.x + canvas_size.x) {
            draw_list->AddLine(ImVec2(line_x, canvas_pos.y + 10.0f), ImVec2(line_x, canvas_pos.y + canvas_size.y), IM_COL32(180, 180, 180, 255));

            std::string bar_str = std::to_string(b + 1);
            if (line_x >= canvas_pos.x) {
                draw_list->AddText(ImVec2(line_x + 4.0f, canvas_pos.y + 2.0f), IM_COL32(200, 200, 200, 230), bar_str.c_str());
            }

            for (int beat = 1; beat < 4; ++beat) {
                const double source_beat_tick = source_bar_tick + beat * ppq;
                const double target_beat_tick = SourceToTargetTick(app.project, source_beat_tick);
                float beat_x = canvas_pos.x + static_cast<float>(target_beat_tick - app.view.scroll_tick) * app.view.px_per_tick;
                if (beat_x >= canvas_pos.x && beat_x <= canvas_pos.x + canvas_size.x) {
                    draw_list->AddLine(ImVec2(beat_x, canvas_pos.y + 16.0f), ImVec2(beat_x, canvas_pos.y + canvas_size.y), IM_COL32(100, 100, 100, 255));
                }
            }
        }
    }

    float playhead_x = canvas_pos.x + static_cast<float>(static_cast<int64_t>(app.transport.current_tick) - static_cast<int64_t>(app.view.scroll_tick)) * app.view.px_per_tick;
    if (playhead_x >= canvas_pos.x && playhead_x <= canvas_pos.x + canvas_size.x) {
        draw_list->AddTriangleFilled(
            ImVec2(playhead_x - 6.0f, canvas_pos.y),
            ImVec2(playhead_x + 6.0f, canvas_pos.y),
            ImVec2(playhead_x, canvas_pos.y + 10.0f),
            IM_COL32(255, 75, 75, 255)
        );
        draw_list->AddLine(
            ImVec2(playhead_x, canvas_pos.y + 10.0f),
            ImVec2(playhead_x, canvas_pos.y + canvas_size.y),
            IM_COL32(255, 75, 75, 255),
            2.0f
        );
    }

    draw_list->PopClipRect();
}

void ClipManager::DrawTrackTimeline(gsr::App& app, Model::Track& track, size_t track_index, float row_height) {
    const uint32_t ppq = app.project.ppq;
    const uint32_t ticks_per_bar = ppq * 4;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(std::max(ImGui::GetContentRegionAvail().x, 50.0f), row_height);

    ImGui::InvisibleButton("TimelineCanvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
    
    ImGuiIO& io = ImGui::GetIO();
    const bool is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    const bool is_clicking = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);

    // Zoom & Horizontal Pan Controls
    if (is_hovered) {
        if (io.KeyShift && io.MouseWheel != 0.0f) {
            float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
            app.view.px_per_tick = std::clamp(app.view.px_per_tick * zoom_factor, 0.005f, 0.2f);
            io.MouseWheel = 0.0f;
            io.MouseWheelH = 0.0f;
        }
        if (io.KeyAlt && io.MouseDelta.x != 0.0f) {
            int64_t delta_ticks = static_cast<int64_t>(io.MouseDelta.x / app.view.px_per_tick);
            int64_t new_scroll = static_cast<int64_t>(app.view.scroll_tick) - delta_ticks;
            app.view.scroll_tick = static_cast<uint64_t>(std::max<int64_t>(0, new_scroll));
        }
    }

    const float px_per_tick = app.view.px_per_tick;
    const uint64_t scroll_tick = app.view.scroll_tick;

    ImVec2 mouse_pos = io.MousePos;
    float rel_mouse_x = mouse_pos.x - canvas_pos.x;
    int64_t hovered_tick = std::max<int64_t>(0, static_cast<int64_t>(rel_mouse_x / px_per_tick) + scroll_tick);
    uint32_t hovered_bar = static_cast<uint32_t>(hovered_tick / ticks_per_bar);

    auto& sel = app.view.cell_selection;

    // Direct Mouse Click Selection Handler (Ignore selection drag when Alt-panning)
    if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyAlt) {
        for (auto& t : app.project.tracks) {
            for (auto& c : t.clips) c.selected = false;
        }

        sel.track_index = static_cast<int>(track_index);
        sel.drag_anchor_bar = hovered_bar;
        sel.start_bar = hovered_bar;
        sel.num_bars = 1;
        sel.active = true;
        app.view.active_track_index = static_cast<int>(track_index);

        for (auto& clip : track.clips) {
            const double clip_target_start = SourceToTargetTick(app.project, clip.start_tick);
            const double clip_target_end = SourceToTargetTick(app.project, clip.start_tick + clip.duration);
            float c_x1 = canvas_pos.x + static_cast<float>(clip_target_start - scroll_tick) * px_per_tick;
            float c_x2 = canvas_pos.x + static_cast<float>(clip_target_end - scroll_tick) * px_per_tick;

            if (mouse_pos.x >= c_x1 && mouse_pos.x <= c_x2) {
                clip.selected = true;
                break;
            }
        }
    } else if (is_clicking && sel.active && sel.track_index == static_cast<int>(track_index) && !io.KeyAlt) {
        uint32_t min_bar = std::min(sel.drag_anchor_bar, hovered_bar);
        uint32_t max_bar = std::max(sel.drag_anchor_bar, hovered_bar);
        sel.start_bar = min_bar;
        sel.num_bars = (max_bar - min_bar) + 1;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

    // Draw Bar Grid
    int start_bar = static_cast<int>(scroll_tick / ticks_per_bar);
    int end_bar = start_bar + static_cast<int>(canvas_size.x / (ticks_per_bar * px_per_tick)) + 2;

    for (int b = start_bar; b <= end_bar; ++b) {
        uint64_t bar_tick = static_cast<uint64_t>(b) * ticks_per_bar;
        float line_x = canvas_pos.x + static_cast<float>(static_cast<int64_t>(bar_tick) - static_cast<int64_t>(scroll_tick)) * px_per_tick;

        if (b % 2 == 0) {
            float next_line_x = line_x + (ticks_per_bar * px_per_tick);
            draw_list->AddRectFilled(
                ImVec2(line_x, canvas_pos.y),
                ImVec2(next_line_x, canvas_pos.y + canvas_size.y),
                IM_COL32(255, 255, 255, 6)
            );
        }

        draw_list->AddLine(
            ImVec2(line_x, canvas_pos.y),
            ImVec2(line_x, canvas_pos.y + canvas_size.y),
            IM_COL32(200, 200, 200, 30),
            1.0f
        );
    }

    // Draw Selected Area Highlight
    if (sel.active && sel.track_index == static_cast<int>(track_index)) {
        float sel_x1 = canvas_pos.x + static_cast<float>(static_cast<int64_t>(sel.start_bar * ticks_per_bar) - static_cast<int64_t>(scroll_tick)) * px_per_tick;
        float sel_x2 = sel_x1 + (sel.num_bars * ticks_per_bar * px_per_tick);

        draw_list->AddRectFilled(
            ImVec2(std::max(sel_x1, canvas_pos.x), canvas_pos.y),
            ImVec2(std::min(sel_x2, canvas_pos.x + canvas_size.x), canvas_pos.y + canvas_size.y),
            IM_COL32(100, 180, 255, 60)
        );
        draw_list->AddRect(
            ImVec2(std::max(sel_x1, canvas_pos.x), canvas_pos.y),
            ImVec2(std::min(sel_x2, canvas_pos.x + canvas_size.x), canvas_pos.y + canvas_size.y),
            IM_COL32(120, 200, 255, 200),
            0.0f, 0, 2.0f
        );
    }

    // Draw Track Clips
    for (const auto& clip : track.clips) {
        const double clip_target_start = SourceToTargetTick(app.project, clip.start_tick);
        const double clip_target_end = SourceToTargetTick(app.project, clip.start_tick + clip.duration);
        float clip_x1 = canvas_pos.x + static_cast<float>(clip_target_start - scroll_tick) * px_per_tick;
        float clip_x2 = canvas_pos.x + static_cast<float>(clip_target_end - scroll_tick) * px_per_tick;

        if (clip_x2 < canvas_pos.x || clip_x1 > canvas_pos.x + canvas_size.x) continue;

        ImU32 fill_color = (clip.type == Model::ClipType::Repeat) ? IM_COL32(160, 90, 180, 200) : IM_COL32(40, 120, 180, 200);
        ImU32 border_color = clip.selected ? IM_COL32(255, 230, 100, 255) : ((clip.type == Model::ClipType::Repeat) ? IM_COL32(210, 140, 240, 255) : IM_COL32(80, 180, 240, 255));

        draw_list->AddRectFilled(
            ImVec2(std::max(clip_x1, canvas_pos.x), canvas_pos.y + 2.0f),
            ImVec2(std::min(clip_x2, canvas_pos.x + canvas_size.x), canvas_pos.y + canvas_size.y - 2.0f),
            fill_color, 4.0f
        );
        draw_list->AddRect(
            ImVec2(std::max(clip_x1, canvas_pos.x), canvas_pos.y + 2.0f),
            ImVec2(std::min(clip_x2, canvas_pos.x + canvas_size.x), canvas_pos.y + canvas_size.y - 2.0f),
            border_color, 4.0f, 0, clip.selected ? 2.5f : 1.5f
        );

        std::string clip_label = (clip.type == Model::ClipType::Repeat ? "[:||] " : "") + clip.name;
        if (clip_x1 + 6.0f < canvas_pos.x + canvas_size.x) {
            draw_list->AddText(ImVec2(std::max(clip_x1 + 6.0f, canvas_pos.x + 4.0f), canvas_pos.y + 4.0f), IM_COL32(255, 255, 255, 230), clip_label.c_str());
        }

        for (const auto& note : clip.notes) {
            const double note_target_start = SourceToTargetTick(app.project, clip.start_tick + note.start_tick);
            const double note_target_end = SourceToTargetTick(app.project, clip.start_tick + note.start_tick + note.duration);
            float n_x1 = canvas_pos.x + static_cast<float>(note_target_start - scroll_tick) * px_per_tick;
            float n_x2 = canvas_pos.x + static_cast<float>(note_target_end - scroll_tick) * px_per_tick;
            float norm_p = static_cast<float>(note.pitch) / 127.0f;
            float n_y = (canvas_pos.y + canvas_size.y - 8.0f) - norm_p * (canvas_size.y - 20.0f);

            draw_list->AddRectFilled(ImVec2(n_x1, n_y), ImVec2(n_x2, n_y + 3.0f), IM_COL32(255, 220, 100, 220));
        }
    }

    // Playhead Drawing
    float playhead_x = canvas_pos.x + static_cast<float>(static_cast<int64_t>(app.transport.current_tick) - static_cast<int64_t>(scroll_tick)) * px_per_tick;
    if (playhead_x >= canvas_pos.x && playhead_x <= canvas_pos.x + canvas_size.x) {
        draw_list->AddLine(ImVec2(playhead_x, canvas_pos.y), ImVec2(playhead_x, canvas_pos.y + canvas_size.y), IM_COL32(255, 75, 75, 255), 2.0f);
    }

    draw_list->PopClipRect();

    TimelineEditorContext ctx{ app, track, track_index, ticks_per_bar };

    // Execute shortcuts ONLY if the timeline window is in focus AND this is the active track
    if (app.view.active_track_index == static_cast<int>(track_index)) {
        m_interaction.HandleKeyboardShortcuts(ctx);
    }

    // Render Context Menu
    if (ImGui::BeginPopupContextItem("TimelineCellContextMenu")) {
        m_interaction.DrawContextMenu(ctx);
        ImGui::EndPopup();
    }
}

} // namespace gsr::gui