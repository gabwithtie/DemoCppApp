#pragma once

#include "ComponentCallbacks.hpp"
#include "../../../model/Train.hpp"
#include <imgui.h>
#include <functional>
#include <string>

namespace gsr::train_components {
    inline void DrawFurnitureCreationPopup(
        Model::FurnitureSlot& slot,
        std::size_t carriage_index,
        std::size_t slot_index,
        const ComponentCallbacks& callbacks) {
        if (!ImGui::BeginPopup("FurnitureCreationPopup")) return;

        ImGui::TextUnformatted("ADD FURNITURE");
        ImGui::Separator();
        if (ImGui::MenuItem("Passenger set") && callbacks.can_add_furniture(carriage_index, slot_index, FURNITURE_ID_PASSENGERSET)) {
            callbacks.add_furniture(carriage_index, slot_index, FURNITURE_ID_PASSENGERSET);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Cargo shelf") && callbacks.can_add_furniture(carriage_index, slot_index, FURNITURE_ID_CARGOSHELF)) {
            callbacks.add_furniture(carriage_index, slot_index, FURNITURE_ID_CARGOSHELF);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
