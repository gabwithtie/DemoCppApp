#include "PowerTerminal.h"
#include "trainsim/grid/GridManager.h"
#include <algorithm>

namespace Trainsim {
    void PowerTerminal::Update(GridCell* cell) {
        if (!cell) return;

        // Ensure construction is complete before functioning
        if (cell->data.find("health") == cell->data.end()) cell->data["health"] = 0.0f;
        if (cell->data["health"] < 1.0f) return;

        // Ensure connection state exists and check it
        if (cell->data.find("connected") == cell->data.end()) cell->data["connected"] = 0.0f;
        bool isConnected = (cell->data["connected"] > 0.0f);

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        GridData& gridData = manager->GetGridData();
        float totalNetPower = 0.0f;

        // Check 4 adjacent directions
        const int dx[] = { 1, -1, 0, 0 };
        const int dy[] = { 0, 0, 1, -1 };

        // 1. Scan adjacent cells for their net power ONLY if connected to a Main
        if (isConnected) {
            for (int i = 0; i < 4; ++i) {
                GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
                if (neighbor && neighbor->type != "EMPTY") {
                    if (neighbor->data.find("net_power") != neighbor->data.end()) {
                        totalNetPower += neighbor->data["net_power"];
                    }
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
        // If not connected, it cannot distribute power locally, regardless of the global battery.
        float currentPowerState = (isConnected && gridData.globalData["stored_power"] > 0.0f) ? 1.0f : 0.0f;

        for (int i = 0; i < 4; ++i) {
            GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
            if (neighbor && neighbor->type != "EMPTY") {
                neighbor->data["powered"] = currentPowerState;
            }
        }
    }

    int PowerTerminal::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Use a counting system so multiple power mains don't overwrite each other's connections
        if (message.type == "connect_main") {
            if (cell->data.find("connected") == cell->data.end()) cell->data["connected"] = 0.0f;
            cell->data["connected"] += 1.0f;
            return 1; // Success
        }

        if (message.type == "disconnect_main") {
            if (cell->data.find("connected") == cell->data.end()) cell->data["connected"] = 0.0f;
            cell->data["connected"] -= 1.0f;

            // Floor at 0 just in case
            if (cell->data["connected"] < 0.0f) cell->data["connected"] = 0.0f;
            return 1; // Success
        }

        // Fallback to base (handles "destroy", etc.)
        return BuildingBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> PowerTerminal::GetMessageList() const {
        return BuildingBehavior::GetMessageList();
    }
}