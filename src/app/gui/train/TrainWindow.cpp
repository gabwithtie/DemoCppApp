#include "TrainWindow.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>

namespace gsr {
    void TrainWindow::DrawSelf() {
        // Control bar for zoom and carriage count
        ImGui::Text("Zoom:");
        ImGui::SameLine();
        ImGui::SliderFloat("##ZoomSlider", &zoom_level, 0.2f, 3.0f, "%.2fx");
        
        ImGui::SameLine();
        if (ImGui::Button("Reset Zoom")) {
            zoom_level = 1.0f;
        }

        ImGui::SameLine();
        ImGui::SliderInt("Carriages", &carriage_count, 1, 50);

        // Process mouse wheel zoom input when hovering over window
        HandleZoom();

        ImGui::Separator();

        // Horizontal scroll container
        ImGui::BeginChild("TrainScrollCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        HandleDragging();
        DrawCarriages();

        ImGui::EndChild();
    }

    void TrainWindow::HandleZoom() {
        if (IsPointerHere() && ImGui::GetIO().MouseWheel != 0.0f) {
            zoom_level += ImGui::GetIO().MouseWheel * 0.1f;
            zoom_level = std::clamp(zoom_level, 0.2f, 3.0f);
        }
    }

    void TrainWindow::HandleDragging() {
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            ImGui::SetScrollX(ImGui::GetScrollX() - delta.x);
            ImGui::SetScrollY(ImGui::GetScrollY() - delta.y);
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }

    void TrainWindow::DrawCarriages() {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 avail_region = ImGui::GetContentRegionAvail();

        // Base dimensions (scaled by zoom)
        const float base_width = 120.0f;
        const float base_height = 60.0f;
        const float base_gap = 12.0f;

        float width = base_width * zoom_level;
        float height = base_height * zoom_level;
        float gap = base_gap * zoom_level;

        float total_width = (width + gap) * carriage_count;
        
        // Calculate total visual height (carriage height + track spacing)
        float track_offset = 9.0f * zoom_level;
        float total_train_height = height + track_offset;

        // Compute vertical centering offset
        float y_offset = 0.0f;
        if (avail_region.y > total_train_height) {
            y_offset = (avail_region.y - total_train_height) * 0.5f;
        }

        ImVec2 start_pos = ImVec2(cursor_pos.x, cursor_pos.y + y_offset);

        // Expand scroll area matching content size
        float dummy_height = std::max(avail_region.y, total_train_height);
        ImGui::Dummy(ImVec2(total_width, dummy_height));

        // Draw track base line
        ImVec2 track_start = ImVec2(start_pos.x, start_pos.y + height + (5.0f * zoom_level));
        ImVec2 track_end = ImVec2(start_pos.x + total_width, start_pos.y + height + track_offset);
        draw_list->AddRectFilled(track_start, track_end, IM_COL32(100, 100, 100, 255));

        // Render individual carriages horizontally
        for (int i = 0; i < carriage_count; ++i) {
            float x_offset = i * (width + gap);
            ImVec2 p_min = ImVec2(start_pos.x + x_offset, start_pos.y);
            ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);

            // Engine color vs general carriage color
            ImU32 body_color = (i == 0) ? IM_COL32(180, 50, 50, 255) : IM_COL32(50, 110, 180, 255);

            // Draw carriage shape
            draw_list->AddRectFilled(p_min, p_max, body_color, 4.0f * zoom_level);
            draw_list->AddRect(p_min, p_max, IM_COL32(230, 230, 230, 255), 4.0f * zoom_level, 0, 2.0f);

            // Label text scaling
            char label[16];
            snprintf(label, sizeof(label), i == 0 ? "Engine" : "Car %d", i);
            ImVec2 text_size = ImGui::CalcTextSize(label);
            if (text_size.x < width) {
                ImVec2 text_pos = ImVec2(
                    p_min.x + (width - text_size.x) * 0.5f,
                    p_min.y + (height - text_size.y) * 0.5f
                );
                draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), label);
            }

            // Draw coupler linkage
            if (i < carriage_count - 1) {
                ImVec2 coupler_min = ImVec2(p_max.x, p_min.y + (height * 0.6f));
                ImVec2 coupler_max = ImVec2(p_max.x + gap, p_min.y + (height * 0.75f));
                draw_list->AddRectFilled(coupler_min, coupler_max, IM_COL32(60, 60, 60, 255));
            }
        }
    }
}