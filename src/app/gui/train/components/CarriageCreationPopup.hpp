#pragma once

#include "ComponentCallbacks.hpp"
#include "../../../model/Train.hpp"
#include <imgui.h>
#include <functional>
#include <string>

namespace gsr::train_components {
    inline void DrawCarriageCreationPopup(
        const ComponentCallbacks& callbacks) {
        if (!ImGui::BeginPopup("CarriageCreationPopup")) return;

        ImGui::TextUnformatted("ADD CARRIAGE");
        ImGui::Separator();
        if (ImGui::MenuItem("Passenger") && callbacks.can_create_carriage(CARRIAGE_ID_PASSENGER)) {
            callbacks.create_carriage(CARRIAGE_ID_PASSENGER);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Cargo") && callbacks.can_create_carriage(CARRIAGE_ID_CARGO)) {
            callbacks.create_carriage(CARRIAGE_ID_CARGO);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
