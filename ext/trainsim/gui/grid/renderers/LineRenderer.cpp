#include "LineRenderer.h"
#include "trainsim/grid/GridManager.h"
#include <string>

namespace Trainsim {
    void LineRenderer::Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) {
        if (!cell || !drawList) return;

        // Skip rendering connections if zoomed out too far for clarity
        if (zoomLevel < 0.1f) return;

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        float cellWidth = pMax.x - pMin.x;
        float cellHeight = pMax.y - pMin.y;

        // Center of the current rendering cell
        ImVec2 centerPos = ImVec2(
            pMin.x + (cellWidth * 0.5f),
            pMin.y + (cellHeight * 0.5f)
        );

        // Iterate through all sequential connection indices
        int index = 0;
        while (true) {
            std::string keyX = "c::" + std::to_string(index) + "::x";
            std::string keyY = "c::" + std::to_string(index) + "::y";

            auto itX = cell->data.find(keyX);
            auto itY = cell->data.find(keyY);

            if (itX == cell->data.end() || itY == cell->data.end()) {
                break; // Reached the end of the connection list
            }

            int targetX = static_cast<int>(itX->second);
            int targetY = static_cast<int>(itY->second);

            // Calculate the screen-space target by finding the relative grid offset
            // and multiplying it by the scaled cell pixel dimensions
            float deltaGridX = static_cast<float>(targetX - cx);
            float deltaGridY = static_cast<float>(targetY - cy);

            ImVec2 targetScreenPos = ImVec2(
                centerPos.x + (deltaGridX * cellWidth),
                centerPos.y + (deltaGridY * cellHeight)
            );

            // Draw a drop-shadow/stroke line for visibility over grid backgrounds
            drawList->AddLine(centerPos, targetScreenPos, IM_COL32(0, 0, 0, 200), zoomLevel * 4.0f);

            // Draw the inner colored cable line (e.g., bright powerline yellow/cyan)
            drawList->AddLine(centerPos, targetScreenPos, IM_COL32(0, 255, 200, 255), zoomLevel * 2.0f);

            index++;
        }
    }
}