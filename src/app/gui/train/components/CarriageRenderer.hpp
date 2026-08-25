#pragma once

#include "CarriageCreationPopup.hpp"
#include "ComponentCallbacks.hpp"
#include "FurnitureRenderer.hpp"
#include "HeadCarriageRenderer.hpp"
#include "../../../model/Train.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>

namespace gsr::train_components {
    inline const char* CarriageLabel(const std::string& type) {
        if (type == CARRIAGE_ID_CARGO) return "CARGO";
        return "PASSENGER";
    }

    inline void DrawCarriages(
        Model::Train& train,
        float zoom,
        const ComponentCallbacks& callbacks) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = 170.0f * zoom;
        const float height = 112.0f * zoom;
        const float gap = 20.0f * zoom;
        const float label_height = 24.0f * zoom;
        const float total_width = (width + gap) * static_cast<float>(train.carriages.size()) + 48.0f * zoom;
        const float total_height = label_height + height;
        const float vertical_offset = std::max(0.0f, (ImGui::GetContentRegionAvail().y - total_height) * 0.5f);
        const ImVec2 start = ImVec2(origin.x, origin.y + vertical_offset);

        ImGui::Dummy(ImVec2(total_width, std::max(ImGui::GetContentRegionAvail().y, total_height)));

        for (std::size_t index = 0; index < train.carriages.size(); ++index) {
            Model::Carriage& carriage = train.carriages[index];
            const ImVec2 min = ImVec2(start.x + index * (width + gap), start.y + label_height);
            const ImVec2 max = ImVec2(min.x + width, min.y + height);
            const bool is_head = carriage.type == CARRIAGE_ID_HEAD;

            if (is_head) {
                DrawHeadCarriage(draw_list, min, max, zoom);
            } else {
                const ImU32 fill = carriage.type == CARRIAGE_ID_CARGO
                    ? IM_COL32(48, 38, 24, 255) : IM_COL32(24, 42, 48, 255);
                draw_list->AddRectFilled(min, max, fill, 2.0f * zoom);
                draw_list->AddRect(min, max, IM_COL32(85, 220, 170, 255), 2.0f * zoom, 0, 2.0f);
                draw_list->AddRect(
                    ImVec2(min.x + 18.0f * zoom, min.y + 18.0f * zoom),
                    ImVec2(max.x - 18.0f * zoom, max.y - 12.0f * zoom),
                    IM_COL32(90, 115, 120, 255), 1.0f * zoom, 0, 1.0f);
                draw_list->AddText(
                    ImVec2(min.x, start.y + 3.0f * zoom),
                    IM_COL32(135, 255, 205, 255), CarriageLabel(carriage.type));
                const std::size_t slot_count = std::min<std::size_t>(carriage.slots.size(), 8);
                const float slot_width = 52.0f * zoom;
                const float slot_height = 16.0f * zoom;
                const float slot_gap = 4.0f * zoom;
                const float slot_start_x = min.x + 25.0f * zoom;
                const float slot_start_y = min.y + 25.0f * zoom;
                for (std::size_t slot = 0; slot < slot_count; ++slot) {
                    const std::size_t column = slot / 4;
                    const std::size_t row = slot % 4;
                    const ImVec2 slot_position = ImVec2(
                        slot_start_x + column * (slot_width + 13.0f * zoom),
                        slot_start_y + row * (slot_height + slot_gap));
                    DrawFurniture(
                        carriage.slots[slot], index, slot, slot_position,
                        ImVec2(slot_width, slot_height), callbacks);
                }

                ImGui::SetCursorScreenPos(ImVec2(min.x + width - 58.0f * zoom, start.y + 3.0f * zoom));
                ImGui::PushID(static_cast<int>(index));
                if (ImGui::SmallButton("X") && callbacks.can_remove_carriage(index)) callbacks.remove_carriage(index);
                ImGui::PopID();
            }
            if (index + 1 < train.carriages.size())
                draw_list->AddLine(ImVec2(max.x, (min.y + max.y) * 0.5f), ImVec2(max.x + gap, (min.y + max.y) * 0.5f), IM_COL32(85, 220, 170, 255), 2.0f * zoom);
        }

        ImGui::SetCursorScreenPos(ImVec2(start.x + (width + gap) * static_cast<float>(train.carriages.size()), start.y + height * 0.35f));
        ImGui::PushID("AddCarriage");
        if (ImGui::Button("+", ImVec2(32.0f * zoom, 32.0f * zoom))) ImGui::OpenPopup("CarriageCreationPopup");
        DrawCarriageCreationPopup(callbacks);
        ImGui::PopID();
    }
}
