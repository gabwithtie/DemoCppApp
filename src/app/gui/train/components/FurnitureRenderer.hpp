#pragma once

#include "ComponentCallbacks.hpp"
#include "FurnitureCreationPopup.hpp"
#include "FurnitureInspectorPopup.hpp"
#include <imgui.h>
#include <functional>
#include <string>

namespace gsr::train_components {
    inline const char* FurnitureLabel(const std::string& type) {
        if (type == FURNITURE_ID_PASSENGERSET) return "PASSENGER";
        if (type == FURNITURE_ID_CARGOSHELF) return "CARGO";
        return "EMPTY";
    }

    inline void DrawFurniture(
        Model::FurnitureSlot& slot,
        std::size_t carriage_index,
        std::size_t slot_index,
        const ImVec2& position,
        const ImVec2& size,
        const ComponentCallbacks& callbacks) {
        ImGui::SetCursorScreenPos(position);
        ImGui::PushID(static_cast<int>(carriage_index));
        ImGui::PushID(static_cast<int>(slot_index));

        const bool occupied = !slot.type.empty();
        ImGui::PushStyleColor(ImGuiCol_Button, occupied
                ? ImVec4(0.10f, 0.42f, 0.29f, 1.0f)
                : ImVec4(0.08f, 0.12f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.62f, 0.42f, 1.0f));
        const char* label = occupied ? FurnitureLabel(slot.type) : "+ EMPTY";
        if (ImGui::Button(label, size))
            ImGui::OpenPopup(occupied ? "FurnitureInspectorPopup" : "FurnitureCreationPopup");
        ImGui::PopStyleColor(2);

        DrawFurnitureInspectorPopup(slot, carriage_index, slot_index, callbacks);
        DrawFurnitureCreationPopup(slot, carriage_index, slot_index, callbacks);

        ImGui::PopID();
        ImGui::PopID();
    }
}
