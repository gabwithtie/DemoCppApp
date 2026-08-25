#pragma once

#include "../../../model/Train.hpp"
#include <imgui.h>
#include <string>

namespace gsr::train_components {
    inline void DrawHeadCarriage(
        ImDrawList* draw_list,
        const ImVec2& min,
        const ImVec2& max,
        float zoom) {
        draw_list->AddRectFilled(min, max, IM_COL32(22, 28, 35, 255), 2.0f * zoom);
        draw_list->AddRect(min, max, IM_COL32(255, 190, 65, 255), 2.0f * zoom, 0, 2.0f);
        draw_list->AddRect(
            ImVec2(min.x + 18.0f * zoom, min.y + 18.0f * zoom),
            ImVec2(max.x - 18.0f * zoom, max.y - 12.0f * zoom),
            IM_COL32(105, 120, 125, 255), 1.0f * zoom, 0, 1.0f);
        draw_list->AddText(
            ImVec2(min.x, min.y - 21.0f * zoom),
            IM_COL32(255, 190, 65, 255), "HEAD / LOCKED");
    }
}
