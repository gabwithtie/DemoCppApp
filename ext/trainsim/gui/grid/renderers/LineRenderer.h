#pragma once
#include "trainsim/gui/grid/common/CellRenderer.h"

namespace Trainsim {
    class LineRenderer : public CellRenderer {
    public:
        void Render(ImDrawList* drawList, const GridCell* cell, ImVec2 pMin, ImVec2 pMax, float zoomLevel) override;
    };
}