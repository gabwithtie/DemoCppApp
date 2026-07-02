#include "EntityHolder.h"
#include "trainsim/grid/GridManager.h"

namespace Trainsim {
    void EntityHolder::Update(GridCell* cell) {
        if (!cell) return;

        // Automatically position entity at the center of this grid cell if uninitialized
        if (cell->data.find("entity_x") == cell->data.end()) {
            int cx = 0, cy = 0;
            if (GridManager::Get()->FindCellCoordinates(cell, cx, cy)) {
                cell->data["entity_x"] = static_cast<float>(cx);
                cell->data["entity_y"] = static_cast<float>(cy);
            }
        }
    }
}