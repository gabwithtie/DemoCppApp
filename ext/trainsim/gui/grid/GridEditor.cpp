#include "GridEditor.h"
#include "imgui.h"
#include <algorithm>

namespace Trainsim {

    GridEditor::GridEditor(GridManager& gridManager) : manager(gridManager) {
        // Set the base cell scale inside the manager
        manager.SetCellSize(base_cell_size);
    }

    unsigned int GridEditor::GetCellColor(const std::string& type) {
        if (type == std::string(magic_enum::enum_name(BuildingType::EMPTY))) 
            return IM_COL32(35, 35, 35, 255);

        size_t hash = std::hash<std::string>{}(type);
        unsigned char r = static_cast<unsigned char>((hash & 0xFF0000) >> 16) | 0x50;
        unsigned char g = static_cast<unsigned char>((hash & 0x00FF00) >> 8) | 0x50;
        unsigned char b = static_cast<unsigned char>(hash & 0x0000FF) | 0x50;
        return IM_COL32(r, g, b, 255);
    }

    void GridEditor::DrawSelf() {
        // Left Panel: Inspector
        ImGui::BeginChild("InspectorSide", ImVec2(left_pane_width, 0), true);
        DrawInspectorPane();
        ImGui::EndChild();

        // Draggable Splitter
        ImGui::SameLine();
        ImGui::InvisibleButton("v_splitter", ImVec2(5.0f, -1.0f));
        if (ImGui::IsItemActive()) {
            left_pane_width += ImGui::GetIO().MouseDelta.x;
        }
        ImGui::SameLine();

        // Right Panel: Interactive Canvas Visualizer
        ImGui::BeginChild("CanvasSide", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        DrawGridCanvasPane();
        ImGui::EndChild();
    }

    void GridEditor::DrawInspectorPane() {
        GridData& data = manager.GetGridData();

        ImGui::SameLine();
        if (ImGui::Button("Reset View")) {
            zoom_level = 1.0f;
            pan_offset = ImVec2(0.0f, 0.0f);
        }

        ImGui::Separator();
        ImGui::Text("Grid Size: %d x %d", data.width, data.height);
        ImGui::Text("Zoom: %.2fx", zoom_level);

        const auto& selected = manager.GetSelectedCells();
        ImGui::Text("Selected Cells: %zu", selected.size());

        if (selected.empty()) {
            ImGui::TextDisabled("Click/drag to select. Middle-click drag to pan. Scroll to zoom.");
            return;
        }

        ImGui::Separator();
        ImGui::Text("**Selection Inspector**");

        auto [fx, fy] = selected.front();
        GridCell* activeCell = data.GetCell(fx, fy);

        if (activeCell) {
            ImGui::Text("Primary Focus: [%d, %d]", fx, fy);

            if (ImGui::InputText("Cell Type", type_buffer, sizeof(type_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                for (auto [sx, sy] : selected) {
                    if (GridCell* c = data.GetCell(sx, sy)) c->type = type_buffer;
                }
                type_buffer[0] = '\0';
            }
            ImGui::TextDisabled("Current Type: '%s'", activeCell->type.c_str());

            ImGui::Spacing();
            ImGui::Text("**Cell Dictionary Data**");
            if (ImGui::BeginTable("DataTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Key");
                ImGui::TableSetupColumn("Value");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();

                std::string keyToDelete = "";
                for (auto& [key, val] : activeCell->data) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID(key.c_str());
                    if (ImGui::InputFloat("##val", &val)) {
                        for (auto [sx, sy] : selected) {
                            if (GridCell* c = data.GetCell(sx, sy)) c->data[key] = val;
                        }
                    }
                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::Button("Delete")) keyToDelete = key;
                }
                ImGui::EndTable();

                if (!keyToDelete.empty()) {
                    for (auto [sx, sy] : selected) {
                        if (GridCell* c = data.GetCell(sx, sy)) c->data.erase(keyToDelete);
                    }
                }
            }

            ImGui::SetNextItemWidth(100.0f);
            ImGui::InputText("##NewKey", new_data_key, sizeof(new_data_key));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.0f);
            ImGui::InputFloat("##NewVal", &new_data_value);
            ImGui::SameLine();
            if (ImGui::Button("Add Pair") && strlen(new_data_key) > 0) {
                for (auto [sx, sy] : selected) {
                    if (GridCell* c = data.GetCell(sx, sy)) c->data[new_data_key] = new_data_value;
                }
                new_data_key[0] = '\0';
                new_data_value = 0.0f;
            }

            ImGui::Separator();
            ImGui::Text("**Messaging Protocol**");
            ImGui::InputText("Message Type", message_type_buffer, sizeof(message_type_buffer));
            if (ImGui::Button("Send Broadcast Message")) {
                CellMessage msg{ message_type_buffer };
                manager.SendMessageToSelected(msg);
            }
        }
    }

    void GridEditor::DrawGridCanvasPane() {
        GridData& data = manager.GetGridData();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        drawList->AddRect(canvasOrigin, ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + canvasSize.y), IM_COL32(100, 100, 100, 255));

        ImGui::InvisibleButton("canvas_interact", canvasSize);
        bool isHovered = ImGui::IsItemHovered();

        ImVec2 mousePos = ImGui::GetMousePos();
        float localMouseX = mousePos.x - canvasOrigin.x;
        float localMouseY = mousePos.y - canvasOrigin.y;

        if (isHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
            pan_offset.x += ImGui::GetIO().MouseDelta.x;
            pan_offset.y += ImGui::GetIO().MouseDelta.y;
        }

        float scrollDelta = ImGui::GetIO().MouseWheel;
        if (isHovered && scrollDelta != 0.0f) {
            float mouseWorldX = (localMouseX - pan_offset.x) / zoom_level;
            float mouseWorldY = (localMouseY - pan_offset.y) / zoom_level;
            zoom_level += scrollDelta * 0.1f;
            zoom_level = std::clamp(zoom_level, 0.1f, 8.0f);
            pan_offset.x = localMouseX - (mouseWorldX * zoom_level);
            pan_offset.y = localMouseY - (mouseWorldY * zoom_level);
        }

        float current_cell_size = base_cell_size * zoom_level;
        float correctedWorldX = (localMouseX - pan_offset.x) / zoom_level;
        float correctedWorldY = (localMouseY - pan_offset.y) / zoom_level;

        if (isHovered) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                manager.Select(correctedWorldX, correctedWorldY);
            }
            else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                manager.BoxSelect(correctedWorldX, correctedWorldY);
            }
            // Context Trigger detection
            else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                // Contextual edge-case: if right clicking a cell not already selected, force-select it
                bool alreadySelected = false;
                int gridX = static_cast<int>(std::floor(correctedWorldX / base_cell_size));
                int gridY = static_cast<int>(std::floor(correctedWorldY / base_cell_size));
                for (auto [sx, sy] : manager.GetSelectedCells()) {
                    if (sx == gridX && sy == gridY) { alreadySelected = true; break; }
                }
                if (!alreadySelected) {
                    manager.Select(correctedWorldX, correctedWorldY);
                }
                ImGui::OpenPopup("GridCanvasContextMenu");
            }
        }

        // Context Menu Visual Logic
        if (ImGui::BeginPopup("GridCanvasContextMenu")) {
            const auto& selected = manager.GetSelectedCells();
            if (!selected.empty()) {
                auto [fx, fy] = selected.front();
                if (GridCell* focusCell = data.GetCell(fx, fy)) {
                    ImGui::TextDisabled("Context: %s Cells", focusCell->type.c_str());
                    ImGui::Separator();

                    // Dynamically fetch and display all action options supported by this specific system configuration
                    auto compatibleMessages = manager.GetCompatibleMessages(focusCell->type);
                    if (compatibleMessages.empty()) {
                        ImGui::TextDisabled("No valid actions for this type");
                    }
                    else {
                        for (const auto& msgType : compatibleMessages) {
                            if (ImGui::MenuItem(msgType.label.c_str())) {
                                manager.SendMessageToSelected(msgType);
                            }
                        }
                    }
                }
            }
            ImGui::EndPopup();
        }

        // GridEditor.cpp (Inside your DrawGrid() or main layout loop)

        // --- PASS 1: Base Floor and Grid Lines ---
        for (int y = 0; y < data.height; ++y) {
            for (int x = 0; x < data.width; ++x) {
                GridCell* cell = data.GetCell(x, y);
                if (!cell) continue;

                ImVec2 pMin = ImVec2(canvasOrigin.x + pan_offset.x + (x * current_cell_size),
                    canvasOrigin.y + pan_offset.y + (y * current_cell_size));
                ImVec2 pMax = ImVec2(pMin.x + current_cell_size, pMin.y + current_cell_size);

                // Culling
                if (pMax.x < canvasOrigin.x || pMin.x > canvasOrigin.x + canvasSize.x ||
                    pMax.y < canvasOrigin.y || pMin.y > canvasOrigin.y + canvasSize.y) {
                    continue;
                }

                drawList->AddRectFilled(pMin, pMax, GetCellColor(cell->type));

                if (zoom_level > 0.3f) {
                    drawList->AddRect(pMin, pMax, IM_COL32(50, 50, 50, 30));
                }
            }
        }

        // --- PASS 2: Foreground Entities & Renderers ---
        for (int y = 0; y < data.height; ++y) {
            for (int x = 0; x < data.width; ++x) {
                GridCell* cell = data.GetCell(x, y);
                if (!cell || cell->type == "EMPTY") continue; // Skip empty cells for performance

                ImVec2 pMin = ImVec2(canvasOrigin.x + pan_offset.x + (x * current_cell_size),
                    canvasOrigin.y + pan_offset.y + (y * current_cell_size));
                ImVec2 pMax = ImVec2(pMin.x + current_cell_size, pMin.y + current_cell_size);

                // Culling (same as pass 1)
                if (pMax.x < canvasOrigin.x || pMin.x > canvasOrigin.x + canvasSize.x ||
                    pMax.y < canvasOrigin.y || pMin.y > canvasOrigin.y + canvasSize.y) {
                    continue;
                }

                auto rendIt = renderers.find(cell->type);
                if (rendIt != renderers.end() && rendIt->second != nullptr) {
                    rendIt->second->Render(drawList, cell, pMin, pMax, zoom_level);
                }
            }
        }

        // Selection frame highlight calculations pass
        for (const auto& [sx, sy] : manager.GetSelectedCells()) {
            float posX = canvasOrigin.x + pan_offset.x + (sx * current_cell_size);
            float posY = canvasOrigin.y + pan_offset.y + (sy * current_cell_size);
            drawList->AddRect(
                ImVec2(posX, posY),
                ImVec2(posX + current_cell_size, posY + current_cell_size),
                IM_COL32(255, 235, 50, 255),
                0.0f,
                0,
                std::max(1.0f, 1.5f * zoom_level)
            );
        }
    }
}