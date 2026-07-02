#include "BuilderHolderRenderer.h"
#include "trainsim/grid/GridManager.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace Trainsim {
    void BuilderHolderRenderer::Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) {
        if (!cell || !drawList) return;

        float cellWidth = pMax.x - pMin.x;
        float cellHeight = pMax.y - pMin.y;

        // -------------------------------------------------------------
        // PASS 1: Render the Home Structure Label ("B")
        // -------------------------------------------------------------
        if (zoomLevel >= 0.35f) {
            std::string glyph = "B";
            ImFont* currentFont = ImGui::GetFont();
            float dynamicFontSize = ImGui::GetFontSize() * zoomLevel;
            ImVec2 glyphSize = currentFont->CalcTextSizeA(dynamicFontSize, FLT_MAX, 0.0f, glyph.c_str());

            ImVec2 textPos = ImVec2(
                pMin.x + (cellWidth - glyphSize.x) * 0.5f,
                pMin.y + (cellHeight - glyphSize.y) * 0.5f
            );

            drawList->AddText(currentFont, dynamicFontSize, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 220), glyph.c_str());
            drawList->AddText(currentFont, dynamicFontSize, textPos, IM_COL32(100, 200, 255, 255), glyph.c_str());
        }

        // -------------------------------------------------------------
        // PASS 2: Render the Active Mobile Builder Entity (Dot)
        // -------------------------------------------------------------
        if (zoomLevel < 0.15f) return;

        auto itX = cell->data.find("entity_x");
        auto itY = cell->data.find("entity_y");

        if (itX != cell->data.end() && itY != cell->data.end()) {
            GridManager* manager = GridManager::Get();
            if (!manager) return;

            int cx = 0, cy = 0;
            if (manager->FindCellCoordinates(cell, cx, cy)) {

                // 1. Fetch current target grid coordinates from backend
                ImVec2 targetGridPos(itX->second, itY->second);

                // 2. Fetch or initialize visual cached coordinates
                ImVec2 currentGridPos;
                auto posIt = visualPositions.find(cell);

                if (posIt == visualPositions.end()) {
                    currentGridPos = targetGridPos; // Snap immediately on first frame
                }
                else {
                    currentGridPos = posIt->second;

                    // Frame-rate independent exponential smoothing
                    float dt = ImGui::GetIO().DeltaTime;
                    float lerpSpeed = 15.0f; // Increase to make the dot catch up faster, decrease for more floatiness
                    float blend = 1.0f - std::exp(-lerpSpeed * dt);

                    currentGridPos.x += (targetGridPos.x - currentGridPos.x) * blend;
                    currentGridPos.y += (targetGridPos.y - currentGridPos.y) * blend;
                }

                // Save updated visual position for the next frame
                visualPositions[cell] = currentGridPos;

                // 3. Convert mapped visual grid position to absolute screen pixels
                float deltaGridX = currentGridPos.x - static_cast<float>(cx);
                float deltaGridY = currentGridPos.y - static_cast<float>(cy);

                ImVec2 dotCenter = ImVec2(
                    pMin.x + (deltaGridX * cellWidth) + (cellWidth * 0.5f),
                    pMin.y + (deltaGridY * cellHeight) + (cellHeight * 0.5f)
                );

                float dotRadius = std::max(3.0f, cellWidth * 0.12f);

                // Draw outer stroke
                drawList->AddCircleFilled(dotCenter, dotRadius + 1.0f, IM_COL32(20, 20, 20, 255));

                // Determine inner state color
                ImU32 dotColor = IM_COL32(255, 215, 0, 255);
                auto itState = cell->data.find("builder_state");
                if (itState != cell->data.end()) {
                    int state = static_cast<int>(itState->second);
                    if (state == 1)      dotColor = IM_COL32(239, 83, 80, 255);
                    else if (state == 2) dotColor = IM_COL32(76, 175, 80, 255);
                    else if (state == 3) dotColor = IM_COL32(255, 167, 38, 255);
                }

                // Draw inner dot
                drawList->AddCircleFilled(dotCenter, dotRadius, dotColor);
            }
        }
    }
}