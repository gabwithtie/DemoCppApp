#include "TerminalMapWindow.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace app {

    TerminalMapWindow::TerminalMapWindow(GameSimulation& simulation) : m_sim(simulation) {
        is_open = true;
    }

    ImVec2 TerminalMapWindow::WorldToScreen(const gbe::Vector2& world_pos, const ImVec2& canvas_origin) const {
        return ImVec2(
            canvas_origin.x + m_pan.x + world_pos.x * m_zoom,
            canvas_origin.y + m_pan.y + world_pos.y * m_zoom
        );
    }

    void TerminalMapWindow::DrawSelf() {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        if (m_pan.x == 0.0f && m_pan.y == 0.0f) {
            m_pan = gbe::Vector2(canvas_sz.x * 0.5f, canvas_sz.y * 0.5f);
        }

        ImGui::InvisibleButton("terminal_canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        const bool is_hovered = ImGui::IsItemHovered();

        if (ImGui::IsItemActive() && (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))) {
            m_pan.x += io.MouseDelta.x;
            m_pan.y += io.MouseDelta.y;
        }

        if (is_hovered && io.MouseWheel != 0.0f) {
            ImVec2 mouse_world = ImVec2((io.MousePos.x - canvas_p0.x - m_pan.x) / m_zoom, (io.MousePos.y - canvas_p0.y - m_pan.y) / m_zoom);
            m_zoom = std::clamp(m_zoom * ((io.MouseWheel > 0.0f) ? 1.15f : (1.0f / 1.15f)), 0.2f, 4.0f);
            m_pan.x = (io.MousePos.x - canvas_p0.x) - mouse_world.x * m_zoom;
            m_pan.y = (io.MousePos.y - canvas_p0.y) - mouse_world.y * m_zoom;
        }

        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(20, 22, 26, 255));

        // Lines
        for (const auto& line : m_sim.GetLines()) {
            for (size_t i = 0; i + 1 < line.station_ids.size(); ++i) {
                const auto* stA = m_sim.FindStation(line.station_ids[i]);
                const auto* stB = m_sim.FindStation(line.station_ids[i + 1]);
                if (stA && stB) {
                    draw_list->AddLine(WorldToScreen(stA->position, canvas_p0), WorldToScreen(stB->position, canvas_p0), line.color, std::max(2.0f, 5.0f * m_zoom));
                }
            }
        }

        // Stations Selection
        const std::string& selected_st_id = m_sim.GetSelectedStationId();
        for (const auto& st : m_sim.GetStations()) {
            ImVec2 p = WorldToScreen(st.position, canvas_p0);
            float r = std::max(4.0f, 8.0f * m_zoom);

            if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                float dx = io.MousePos.x - p.x;
                float dy = io.MousePos.y - p.y;
                if ((dx * dx + dy * dy) <= (r + 6.0f) * (r + 6.0f)) {
                    m_sim.SelectStation(st.id);
                }
            }

            if (st.id == selected_st_id) {
                draw_list->AddCircle(p, r + 5.0f, IM_COL32(0, 255, 200, 255), 0, 2.5f);
            }

            draw_list->AddCircleFilled(p, r, IM_COL32(255, 255, 255, 255));
            draw_list->AddCircle(p, r, IM_COL32(0, 0, 0, 255), 0, 2.0f);
            draw_list->AddText(ImVec2(p.x + r + 4, p.y - 6), IM_COL32(220, 220, 220, 255), st.name.c_str());
        }

        // Dynamic Trains Selection
        const std::string& selected_train_id = m_sim.GetSelectedTrainId();
        for (const auto& train : m_sim.GetTrains()) {
            gbe::Vector2 world_pos = m_sim.GetTrainWorldPosition(train);
            ImVec2 p = WorldToScreen(world_pos, canvas_p0);
            float r = std::max(4.0f, 8.0f * m_zoom);

            // Train Hit-testing on Left Click
            if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (io.MousePos.x >= p.x - r - 4.0f && io.MousePos.x <= p.x + r + 4.0f &&
                    io.MousePos.y >= p.y - r - 4.0f && io.MousePos.y <= p.y + r + 4.0f) {
                    m_sim.SelectTrain(train.id);
                }
            }

            // Active Route Overlay
            if (train.active_path.size() >= 2) {
                for (size_t i = 0; i + 1 < train.active_path.size(); ++i) {
                    const auto* stA = m_sim.FindStation(train.active_path[i]);
                    const auto* stB = m_sim.FindStation(train.active_path[i + 1]);
                    if (stA && stB) {
                        draw_list->AddLine(WorldToScreen(stA->position, canvas_p0), WorldToScreen(stB->position, canvas_p0), IM_COL32(255, 255, 0, 100), std::max(3.0f, 8.0f * m_zoom));
                    }
                }
            }

            // Selected Train Highlight Box
            if (train.id == selected_train_id) {
                draw_list->AddRect(ImVec2(p.x - r - 5.0f, p.y - r - 5.0f), ImVec2(p.x + r + 5.0f, p.y + r + 5.0f), IM_COL32(0, 255, 200, 255), 3.0f, 0, 2.0f);
            }

            draw_list->AddRectFilled(ImVec2(p.x - r, p.y - r), ImVec2(p.x + r, p.y + r), IM_COL32(255, 200, 40, 255), 3.0f);
            draw_list->AddRect(ImVec2(p.x - r, p.y - r), ImVec2(p.x + r, p.y + r), IM_COL32(0, 0, 0, 255), 3.0f, 0, 1.5f);
            draw_list->AddText(ImVec2(p.x - r, p.y - r - 14), IM_COL32(255, 200, 40, 255), train.id.c_str());
        }

        draw_list->PopClipRect();
    }

} // namespace app