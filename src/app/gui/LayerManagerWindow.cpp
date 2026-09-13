#include "LayerManagerWindow.hpp"

#include "../App.hpp"

#include <algorithm>
#include <imgui.h>
#include <string>

namespace {

static std::string DefaultLayerName(size_t idx) {
    return "Layer " + std::to_string(idx + 1);
}

} // namespace

namespace app {

LayerManagerWindow::LayerManagerWindow() = default;

void LayerManagerWindow::DrawSelf() {
    App& app = App::GetInstance();
    Model::Project& project = app.project;

    if (project.w <= 0 || project.h <= 0) {
        ImGui::TextUnformatted("Project dimensions are invalid.");
        return;
    }

    if (project.layers.empty()) {
        if (ImGui::Button("Create Base Layer")) {
            app.edit_history.Execute(app.project, "Create Base Layer", [&](Model::Project& p) {
                p.layers.emplace_back();
                p.layers.back().name = "Background";
                p.layers.back().Resize(p.w, p.h, Model::Layer::Transparent);
            });
            app.selected_layer_index = 0;
        }
        ImGui::TextUnformatted("No layers in this project yet.");
        return;
    }

    app.selected_layer_index = std::clamp(app.selected_layer_index, 0, static_cast<int>(project.layers.size()) - 1);

    if (ImGui::Button("Add Layer")) {
        int newIndex = 0;
        app.edit_history.Execute(app.project, "Add Layer", [&](Model::Project& p) {
            p.layers.emplace_back();
            Model::Layer& newLayer = p.layers.back();
            newLayer.name = DefaultLayerName(p.layers.size() - 1);
            newLayer.Resize(p.w, p.h, Model::Layer::Transparent);
            newIndex = static_cast<int>(p.layers.size()) - 1;
        });
        app.selected_layer_index = newIndex;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Selected") && !project.layers.empty()) {
        const int eraseIndex = app.selected_layer_index;
        app.edit_history.Execute(app.project, "Delete Layer", [&](Model::Project& p) {
            p.layers.erase(p.layers.begin() + eraseIndex);
        });
        if (project.layers.empty()) {
            app.selected_layer_index = 0;
            return;
        }
        app.selected_layer_index = std::clamp(app.selected_layer_index, 0, static_cast<int>(project.layers.size()) - 1);
    }

    ImGui::Separator();

    ImGui::BeginChild("LayerList", ImVec2(0.0f, 220.0f), true);
    for (int i = 0; i < static_cast<int>(project.layers.size()); ++i) {
        Model::Layer& layer = project.layers[static_cast<size_t>(i)];
        layer.EnsureSize(project.w, project.h, Model::Layer::Transparent);

        if (layer.name.empty()) {
            layer.name = DefaultLayerName(static_cast<size_t>(i));
        }

        ImGui::PushID(i);
        bool visible = layer.visible;
        if (ImGui::Checkbox("##visible", &visible)) {
            app.edit_history.Execute(app.project, "Toggle Layer Visibility", [&](Model::Project& p) {
                if (i >= 0 && i < static_cast<int>(p.layers.size())) {
                    p.layers[static_cast<size_t>(i)].visible = visible;
                }
            });
        }
        ImGui::SameLine();
        if (ImGui::Selectable(layer.name.c_str(), i == app.selected_layer_index)) {
            app.selected_layer_index = i;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    Model::Layer& selected = project.layers[static_cast<size_t>(app.selected_layer_index)];

    char nameBuffer[128]{};
    const std::string currentName = selected.name;
    const size_t copyCount = std::min(currentName.size(), sizeof(nameBuffer) - 1);
    std::copy_n(currentName.c_str(), copyCount, nameBuffer);

    if (ImGui::InputText("Layer Name", nameBuffer, sizeof(nameBuffer))) {
        const int selectedIndex = app.selected_layer_index;
        const std::string newName(nameBuffer);
        app.edit_history.Execute(app.project, "Rename Layer", [&](Model::Project& p) {
            if (selectedIndex >= 0 && selectedIndex < static_cast<int>(p.layers.size())) {
                p.layers[static_cast<size_t>(selectedIndex)].name = newName;
            }
        });
    }

    if (ImGui::Button("Clear Layer")) {
        const int selectedIndex = app.selected_layer_index;
        app.edit_history.Execute(app.project, "Clear Layer", [&](Model::Project& p) {
            if (selectedIndex >= 0 && selectedIndex < static_cast<int>(p.layers.size())) {
                p.layers[static_cast<size_t>(selectedIndex)].Clear(Model::Layer::Transparent);
            }
        });
    }

    ImGui::Text("Pixels: %zu", selected.pixels.size());
}

} // namespace app
