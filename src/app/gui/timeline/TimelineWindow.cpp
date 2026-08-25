#include "TimelineWindow.hpp"
#include <imgui.h>
#include <string>
#include <algorithm>
#include <cmath>

#include "util/TimelineWarp.hpp"

namespace gsr::gui {

TimelineWindow::TimelineWindow(gsr::App& app)
    : m_app(app) {}

void TimelineWindow::DrawSelf() {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_app.view.cell_selection = {};
    }

    const float header_width = ImGui::GetWindowContentRegionMax().x -
                               ImGui::GetWindowContentRegionMin().x;
    float header_line_width = 110.0f;
    const auto place_header_item = [&](float estimated_width) {
        constexpr float ITEM_SPACING = 8.0f;
        if (header_line_width + ITEM_SPACING + estimated_width > header_width) {
            ImGui::NewLine();
            header_line_width = estimated_width;
        } else {
            ImGui::SameLine();
            header_line_width += ITEM_SPACING + estimated_width;
        }
    };

    // Top Controls Bar
    if (ImGui::Button("+ Add Track")) {
        m_app.SaveUndoPoint();
        Model::Track new_track;
        new_track.name = "Track " + std::to_string(m_app.project.tracks.size() + 1);
        m_app.project.tracks.push_back(new_track);
        m_app.view.active_track_index = static_cast<int>(m_app.project.tracks.size() - 1);
    }

    place_header_item(110.0f);

    // Default Tempo Side Input (Active only when no time keys exist)
    if (m_app.project.time_keys.empty()) {
        ImGui::SetNextItemWidth(80.0f);
        if (ImGui::InputDouble("Default BPM", &m_app.project.default_bpm, 1.0, 5.0, "%.1f")) {
            RecalculateTempoFromTimeKeys(m_app.project, m_app.transport);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            m_app.SaveUndoPoint();
        }
    } else {
        ImGui::TextDisabled("BPM: Dynamic (Warp Keys Active)");
    }

    place_header_item(190.0f);
    ImGui::TextDisabled("| Total Tracks: %zu", m_app.project.tracks.size());

    place_header_item(140.0f);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Track Height", &m_app.view.row_height, 24.0f, 160.0f, "%.0f px");

    place_header_item(90.0f);
    if (ImGui::Button("Columns")) {
        ImGui::OpenPopup("TimelineColumns");
    }
    if (ImGui::BeginPopup("TimelineColumns")) {
        ImGui::TextUnformatted("Visible columns");
        ImGui::Checkbox("M/S", &m_app.view.show_mute_solo_columns);
        ImGui::Checkbox("Volume", &m_app.view.show_volume_column);
        ImGui::Checkbox("Pan", &m_app.view.show_pan_column);
        ImGui::EndPopup();
    }

    place_header_item(140.0f);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Zoom", &m_app.view.px_per_tick, 0.01f, 0.2f, "%.3f px/t");

    place_header_item(170.0f);
    int scroll_bar_pos = static_cast<int>(m_app.view.scroll_tick / m_app.project.ppq);
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::SliderInt("Scroll (Beats)", &scroll_bar_pos, 0, 500)) {
        m_app.view.scroll_tick = static_cast<uint64_t>(scroll_bar_pos) * m_app.project.ppq;
    }

    ImGui::Separator();

    constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerH |
                                           ImGuiTableFlags_RowBg |
                                           ImGuiTableFlags_Resizable |
                                           ImGuiTableFlags_Hideable;

    if (ImGui::BeginTable("TimelineTracks", 5, table_flags)) {
        ImGui::TableSetupColumn("Track Name", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("M/S", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Volume", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Pan", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Timeline / Clip View", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetColumnEnabled(1, m_app.view.show_mute_solo_columns);
        ImGui::TableSetColumnEnabled(2, m_app.view.show_volume_column);
        ImGui::TableSetColumnEnabled(3, m_app.view.show_pan_column);
        ImGui::TableHeadersRow();
        
        // --- Time Key Bar Row ---
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Time Keys");
        
        ImGui::TableSetColumnIndex(4);
        DrawTimeKeyBar();

        // --- Ruler Row ---
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 28.0f);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("Ruler / Loop");
        ImGui::TableSetColumnIndex(4);
        m_clip_manager.DrawRuler(m_app, 28.0f);

        // --- Tracks View ---
        for (size_t i = 0; i < m_app.project.tracks.size(); ++i) {
            auto& track = m_app.project.tracks[i];
            ImGui::PushID(static_cast<int>(i));

            ImGui::TableNextRow(ImGuiTableRowFlags_None, m_app.view.row_height);

            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(24.0f);
            if (ImGui::SmallButton("X")) {
                m_app.SaveUndoPoint();
                m_app.project.tracks.erase(m_app.project.tracks.begin() + static_cast<std::ptrdiff_t>(i));

                const int deleted_track_index = static_cast<int>(i);
                if (m_app.view.cell_selection.track_index == deleted_track_index) {
                    m_app.view.cell_selection.track_index = -1;
                    m_app.view.cell_selection.active = false;
                } else if (m_app.view.cell_selection.track_index > deleted_track_index) {
                    --m_app.view.cell_selection.track_index;
                }

                if (m_app.project.tracks.empty()) {
                    m_app.view.active_track_index = -1;
                } else if (m_app.view.active_track_index > deleted_track_index) {
                    --m_app.view.active_track_index;
                } else if (m_app.view.active_track_index == deleted_track_index) {
                    m_app.view.active_track_index = std::min(
                        static_cast<int>(i), static_cast<int>(m_app.project.tracks.size() - 1));
                }

                ImGui::PopID();
                break;
            }
            ImGui::SameLine();
            const bool is_active = (m_app.view.active_track_index == static_cast<int>(i));
            if (ImGui::Selectable(track.name.c_str(), is_active, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                m_app.view.active_track_index = static_cast<int>(i);
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox("M", &track.muted);
            ImGui::SameLine();
            ImGui::Checkbox("S", &track.solo);

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderFloat("##Vol", &track.volume, 0.0f, 1.0f, "%.2f");

            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderFloat("##Pan", &track.pan, -1.0f, 1.0f, "%.2f");

            ImGui::TableSetColumnIndex(4);
            m_clip_manager.DrawTrackTimeline(m_app, track, i, m_app.view.row_height);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void TimelineWindow::DrawTimeKeyBar() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = 22.0f;

    // Background highlight bar
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(32, 32, 36, 255));

    // Submit single canvas button for unified interaction
    ImGui::InvisibleButton("##TimeKeyBarCanvas", canvas_size);
    bool canvas_hovered = ImGui::IsItemHovered();
    bool canvas_active = ImGui::IsItemActive();
    bool canvas_activated = ImGui::IsItemActivated();
    bool canvas_deactivated = ImGui::IsItemDeactivated();

    float px_per_tick = m_app.view.px_per_tick;
    uint64_t scroll_tick = m_app.view.scroll_tick;
    ImVec2 mouse_pos = ImGui::GetMousePos();

    // 1. Hit-test key handles under cursor
    int hovered_key_index = -1;
    constexpr float HIT_RADIUS_X = 8.0f;

    for (size_t i = 0; i < m_app.project.time_keys.size(); ++i) {
        float handle_x = canvas_pos.x + static_cast<float>(int64_t(m_app.project.time_keys[i].target_tick) - int64_t(scroll_tick)) * px_per_tick;
        
        if (std::abs(mouse_pos.x - handle_x) <= HIT_RADIUS_X) {
            hovered_key_index = static_cast<int>(i);
            break;
        }
    }

    // Set EW cursor feedback when hovering or dragging
    if (hovered_key_index != -1 || m_dragging_key != nullptr) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    // 2. Begin Dragging
    if (canvas_activated && hovered_key_index != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        m_app.SaveUndoPoint();
        m_dragging_key = &m_app.project.time_keys[hovered_key_index];
    }

    // 3. Update active drag position
    if (canvas_active && m_dragging_key != nullptr) {
        float mouse_x = mouse_pos.x - canvas_pos.x;
        int64_t new_target = static_cast<int64_t>(scroll_tick) + static_cast<int64_t>(mouse_x / px_per_tick);
        m_dragging_key->target_tick = static_cast<uint64_t>(std::max<int64_t>(0, new_target));
        
        RecalculateTempoFromTimeKeys(m_app.project, m_app.transport);
    }

    // 4. End Dragging & finalize chronological order
    if (canvas_deactivated || !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (m_dragging_key != nullptr) {
            m_dragging_key = nullptr;
            RecalculateTempoFromTimeKeys(m_app.project, m_app.transport);
        }
    }

    // 5. Delete key on right click
    if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hovered_key_index != -1) {
        m_app.SaveUndoPoint();
        m_app.project.time_keys.erase(m_app.project.time_keys.begin() + hovered_key_index);
        m_dragging_key = nullptr;
        RecalculateTempoFromTimeKeys(m_app.project, m_app.transport);
    }
    // 6. Double click empty bar area to create key
    else if (canvas_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered_key_index == -1) {
        m_app.SaveUndoPoint();
        float mouse_x = mouse_pos.x - canvas_pos.x;
        uint64_t clicked_tick = scroll_tick + static_cast<uint64_t>(std::max(0.0f, mouse_x / px_per_tick));

        const uint64_t source_tick = static_cast<uint64_t>(std::max(
            0.0, TargetToSourceTick(m_app.project, static_cast<double>(clicked_tick))));
        m_app.project.time_keys.push_back({ source_tick, clicked_tick, false });
        RecalculateTempoFromTimeKeys(m_app.project, m_app.transport);
    }

    // 7. Render markers
    for (size_t i = 0; i < m_app.project.time_keys.size(); ++i) {
        auto& key = m_app.project.time_keys[i];
        float handle_x = canvas_pos.x + static_cast<float>(int64_t(key.target_tick) - int64_t(scroll_tick)) * px_per_tick;
        ImVec2 marker_center(handle_x, canvas_pos.y + 11.0f);

        bool is_key_hovered = (hovered_key_index == static_cast<int>(i));
        bool is_key_dragged = (&key == m_dragging_key);

        ImU32 color = is_key_dragged ? IM_COL32(255, 255, 120, 255) :
                      (is_key_hovered ? IM_COL32(255, 220, 100, 255) : IM_COL32(220, 160, 40, 255));

        const ImVec2 diamond[4] = {
            ImVec2(marker_center.x, marker_center.y - 7.0f),
            ImVec2(marker_center.x + 6.0f, marker_center.y),
            ImVec2(marker_center.x, marker_center.y + 7.0f),
            ImVec2(marker_center.x - 6.0f, marker_center.y)
        };

        if (is_key_hovered || is_key_dragged) {
            draw_list->AddConvexPolyFilled(diamond, 4, color);
        } else {
            draw_list->AddPolyline(diamond, 4, color, true, 2.0f);
        }
    }
}

} // namespace gsr::gui