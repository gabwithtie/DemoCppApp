#include "TimelineWindow.hpp"
#include <imgui.h>
#include <string>

namespace gsr::gui {

TimelineWindow::TimelineWindow(gsr::App& app)
    : m_app(app) {}

void TimelineWindow::DrawSelf() {
    // Top Controls Bar
    if (ImGui::Button("+ Add Track")) {
        Model::Track new_track;
        new_track.name = "Track " + std::to_string(m_app.project.tracks.size() + 1);
        m_app.project.tracks.push_back(new_track);
        m_app.view.active_track_index = static_cast<int>(m_app.project.tracks.size() - 1);
    }

    ImGui::SameLine();
    ImGui::TextDisabled("| Total Tracks: %zu", m_app.project.tracks.size());

    ImGui::SameLine(0.0f, 20.0f);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Zoom", &m_app.view.px_per_tick, 0.01f, 0.2f, "%.3f px/t");

    ImGui::SameLine();
    int scroll_bar_pos = static_cast<int>(m_app.view.scroll_tick / m_app.project.ppq);
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::SliderInt("Scroll (Beats)", &scroll_bar_pos, 0, 500)) {
        m_app.view.scroll_tick = static_cast<uint64_t>(scroll_bar_pos) * m_app.project.ppq;
    }

    ImGui::Separator();

    constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerH | 
                                           ImGuiTableFlags_RowBg | 
                                           ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("TimelineTracks", 5, table_flags)) {
        ImGui::TableSetupColumn("Track Name", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("M/S", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Volume", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Pan", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Timeline / Clip View", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // --- Ruler Row ---
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 28.0f);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("Ruler / Loop");
        ImGui::TableSetColumnIndex(4);
        m_clip_manager.DrawRuler(m_app, 28.0f);

        constexpr float ROW_HEIGHT = 48.0f;

        // --- Tracks View ---
        for (size_t i = 0; i < m_app.project.tracks.size(); ++i) {
            auto& track = m_app.project.tracks[i];
            ImGui::PushID(static_cast<int>(i));

            ImGui::TableNextRow(ImGuiTableRowFlags_None, ROW_HEIGHT);

            // 1. Track Name & Row Selection
            ImGui::TableSetColumnIndex(0);
            const bool is_active = (m_app.view.active_track_index == static_cast<int>(i));
            if (ImGui::Selectable(track.name.c_str(), is_active, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                m_app.view.active_track_index = static_cast<int>(i);
            }

            // 2. Mute / Solo
            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox("M", &track.muted);
            ImGui::SameLine();
            ImGui::Checkbox("S", &track.solo);

            // 3. Volume
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderFloat("##Vol", &track.volume, 0.0f, 1.0f, "%.2f");

            // 4. Pan
            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderFloat("##Pan", &track.pan, -1.0f, 1.0f, "%.2f");

            // 5. Clip View (Delegated to ClipManager)
            ImGui::TableSetColumnIndex(4);
            m_clip_manager.DrawTrackTimeline(m_app, track, i, ROW_HEIGHT);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

} // namespace gsr::gui