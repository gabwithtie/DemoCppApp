#include "PowerTerminal.h"
#include "trainsim/grid/GridManager.h"
#include <algorithm>

namespace Trainsim {
    void PowerTerminal::Update(GridCell* cell) {
        if (!cell) return;

        // Ensure construction is complete before functioning
        if (cell->data.find("health") == cell->data.end()) cell->data["health"] = 0.0f;
        if (cell->data["health"] < 1.0f) return;

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        GridData& gridData = manager->GetGridData();
        float totalNetPower = 0.0f;

        // Check 4 adjacent directions
        const int dx[] = { 1, -1, 0, 0 };
        const int dy[] = { 0, 0, 1, -1 };

        // 1. Scan adjacent cells for their net power contribution/draw
        for (int i = 0; i < 4; ++i) {
            GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
            if (neighbor && neighbor->type != "EMPTY") {
                if (neighbor->data.find("net_power") != neighbor->data.end()) {
                    totalNetPower += neighbor->data["net_power"];
                }
            }
        }

        // Initialize global storage if it doesn't exist
        if (gridData.globalData.find("stored_power") == gridData.globalData.end()) {
            gridData.globalData["stored_power"] = 0.0f;
        }

        // 2. Add aggregate power to the global network capacity
        gridData.globalData["stored_power"] += totalNetPower;

        // Prevent negative global capacity bugs
        if (gridData.globalData["stored_power"] <= 0.0f) {
            gridData.globalData["stored_power"] = 0.0f;
        }

        // 3. Assign the active power state back to the adjacent buildings
        float currentPowerState = (gridData.globalData["stored_power"] > 0.0f) ? 1.0f : 0.0f;

        for (int i = 0; i < 4; ++i) {
            GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
            if (neighbor && neighbor->type != "EMPTY") {
                neighbor->data["powered"] = currentPowerState;
            }
        }
    }
}