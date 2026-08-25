#include "TrainWindow.h"
#include "../../App.hpp"
#include "components/CarriageRenderer.hpp"
#include <imgui.h>
#include <algorithm>
#include <utility>

namespace gsr {
    void TrainWindow::DrawSelf() {
        Model::Train& train = App::GetInstance().project.train;
        if (CanCreateHeadCarriage()) {
            Model::Carriage head;
            head.type = CARRIAGE_ID_HEAD;
            if (train.carriages.empty()) train.carriages.push_back(std::move(head));
            else train.carriages.insert(train.carriages.begin(), std::move(head));
        }

        ImGui::TextUnformatted("ZOOM");
        ImGui::SameLine();
        ImGui::SliderFloat("##ZoomSlider", &zoom_level, 0.2f, 3.0f, "%.2fx");
        ImGui::SameLine();
        if (ImGui::Button("RESET")) zoom_level = 1.0f;
        ImGui::SameLine();
        ImGui::TextDisabled("%zu CARRIAGE(S)", train.carriages.size());

        HandleZoom();
        ImGui::Separator();
        ImGui::BeginChild("TrainScrollCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        HandleDragging();
        DrawCarriages();
        ImGui::EndChild();
    }

    void TrainWindow::HandleZoom() {
        if (IsPointerHere() && ImGui::GetIO().MouseWheel != 0.0f)
            zoom_level = std::clamp(zoom_level + ImGui::GetIO().MouseWheel * 0.1f, 0.2f, 3.0f);
    }

    void TrainWindow::HandleDragging() {
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
            const ImVec2 delta = ImGui::GetIO().MouseDelta;
            ImGui::SetScrollX(ImGui::GetScrollX() - delta.x);
            ImGui::SetScrollY(ImGui::GetScrollY() - delta.y);
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }

    void TrainWindow::DrawCarriages() {
        auto& train = App::GetInstance().project.train;
        train_components::ComponentCallbacks callbacks{
            [&](std::size_t index) { return CanRemoveCarriage(index); },
            [&](std::size_t index) { train.carriages.erase(train.carriages.begin() + static_cast<std::ptrdiff_t>(index)); },
            [&](std::size_t carriage, std::size_t slot, const std::string& type) { return CanAddFurniture(carriage, slot, type); },
            [&](std::size_t carriage, std::size_t slot) { return CanRemoveFurniture(carriage, slot); },
            [&](std::size_t carriage, std::size_t slot, const std::string& type) { train.carriages[carriage].slots[slot].type = type; },
            [&](std::size_t carriage, std::size_t slot) { train.carriages[carriage].slots[slot].type.clear(); },
            [&](const std::string& type) { return CanAddCarriage(type); },
            [&](const std::string& type) { AddCarriage(type); }};
        train_components::DrawCarriages(train, zoom_level, callbacks);
    }

    void TrainWindow::AddCarriage(const std::string& type) {
        if (!CanAddCarriage(type)) return;
        Model::Carriage carriage;
        carriage.type = type;
        carriage.slots.resize(8);
        App::GetInstance().project.train.carriages.push_back(std::move(carriage));
    }

    bool TrainWindow::CanCreateHeadCarriage() const {
        const auto& carriages = App::GetInstance().project.train.carriages;
        return carriages.empty() || carriages.front().type != CARRIAGE_ID_HEAD;
    }

    bool TrainWindow::CanAddCarriage(const std::string& type) const {
        return type == CARRIAGE_ID_PASSENGER || type == CARRIAGE_ID_CARGO;
    }

    bool TrainWindow::CanRemoveCarriage(std::size_t index) const {
        const auto& carriages = App::GetInstance().project.train.carriages;
        return index > 0 && index < carriages.size();
    }

    bool TrainWindow::CanAddFurniture(std::size_t carriage, std::size_t slot, const std::string& type) const {
        const auto& carriages = App::GetInstance().project.train.carriages;
        return carriage < carriages.size() && slot < carriages[carriage].slots.size()
            && carriages[carriage].slots[slot].type.empty()
            && (type == FURNITURE_ID_PASSENGERSET || type == FURNITURE_ID_CARGOSHELF);
    }

    bool TrainWindow::CanRemoveFurniture(std::size_t carriage, std::size_t slot) const {
        const auto& carriages = App::GetInstance().project.train.carriages;
        return carriage < carriages.size() && slot < carriages[carriage].slots.size()
            && !carriages[carriage].slots[slot].type.empty();
    }
}
