#pragma once
#include "imgui.h"
#include "trainsim/grid/common/GridCell.h"

namespace Trainsim {
    class CellRenderer {
    public:
        virtual ~CellRenderer() = default;

        // Executes custom ImGui ImDrawList passes on a per-cell canvas boundary footprint
        virtual void Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) = 0;
    };
}