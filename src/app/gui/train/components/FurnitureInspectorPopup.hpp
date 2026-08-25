#pragma once

#include "ComponentCallbacks.hpp"
#include "../../../model/Train.hpp"
#include <imgui.h>
#include <functional>

namespace gsr::train_components {
    inline void DrawFurnitureInspectorPopup(
        Model::FurnitureSlot& slot,
        std::size_t carriage_index,
        std::size_t slot_index,
        const ComponentCallbacks& callbacks) {
        if (!ImGui::BeginPopup("FurnitureInspectorPopup")) return;

        ImGui::TextUnformatted("FURNITURE INSPECTOR");
        ImGui::Separator();
        ImGui::Text("Type: %s", slot.type.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Remove furniture") && callbacks.can_remove_furniture(carriage_index, slot_index)) {
            callbacks.remove_furniture(carriage_index, slot_index);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
