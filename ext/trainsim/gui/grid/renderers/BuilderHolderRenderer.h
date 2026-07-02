#pragma once
#include "trainsim/gui/grid/common/CellRenderer.h"
#include <unordered_map>
#include <imgui.h>

namespace Trainsim {
    class BuilderHolderRenderer : public CellRenderer {
    public:
        void Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) override;

    private:
        // Purely visual tracking for frame-to-frame dot smoothing
        std::unordered_map<const GridCell*, ImVec2> visualPositions;
    };
}