#include "ConveyorRenderer.h"
#include <string>

namespace Trainsim {
    void ConveyorRenderer::Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) {
        // Culling optimization: Skip text rasterization steps if grid units become tiny
        if (zoomLevel < 0.35f) return;

        // Safely extract the direction from the cell's data dictionary (default to North/0)
        float dirVal = 0.0f;
        auto it = cell->data.find("direction");
        if (it != cell->data.end()) {
            dirVal = it->second;
        }

        int dir = static_cast<int>(dirVal);
        std::string glyph;

        // Map numerical orientation indices to visual ASCII arrows
        if (dir == 0)      glyph = "^";
        else if (dir == 1) glyph = ">";
        else if (dir == 2) glyph = "v";
        else if (dir == 3) glyph = "<";
        else               glyph = "?"; // Fallback for invalid states

        // Calculate dynamic dimensions of the bounding container box
        float cellWidth = pMax.x - pMin.x;
        float cellHeight = pMax.y - pMin.y;

        // Query standard ImGui font metrics layout scaling dimensions
        ImFont* currentFont = ImGui::GetFont();
        float dynamicFontSize = ImGui::GetFontSize() * zoomLevel;

        // Contextually match sizing limits to scale cleanly with zoom bounds
        ImVec2 glyphSize = currentFont->CalcTextSizeA(dynamicFontSize, FLT_MAX, 0.0f, glyph.c_str());

        // Center calculation formula
        ImVec2 textPos = ImVec2(
            pMin.x + (cellWidth - glyphSize.x) * 0.5f,
            pMin.y + (cellHeight - glyphSize.y) * 0.5f
        );

        // Render drop shadow pass for legibility against colorful background tiles
        drawList->AddText(currentFont, dynamicFontSize, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 220), glyph.c_str());

        // Render front foreground character layer pass
        drawList->AddText(currentFont, dynamicFontSize, textPos, IM_COL32(255, 255, 255, 255), glyph.c_str());
    }
}