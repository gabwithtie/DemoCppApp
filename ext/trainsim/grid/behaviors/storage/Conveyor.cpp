// Conveyor.cpp
#include "Conveyor.h"
#include "trainsim/grid/GridManager.h"
#include <string>
#include <algorithm>

namespace Trainsim {
    void Conveyor::Update(GridCell* cell) {
        if (!cell) return;

        // Ensure state assertions exist
        if (cell->data.find("max_storage") == cell->data.end()) {
            cell->data["max_storage"] = 4.0f; // Sane capacity for basic items traveling in a line
        }
        if (cell->data.find("direction") == cell->data.end()) {
            cell->data["direction"] = 0.0f; // 0: North, 1: East, 2: South, 3: West
        }

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        // Map facing indices to 2D Cartesian transformation steps
        int dir = static_cast<int>(cell->data["direction"]);
        int dx = 0, dy = 0;

        if (dir == 0)      dy = -1; // North
        else if (dir == 1) dx = 1;  // East
        else if (dir == 2) dy = 1;  // South
        else if (dir == 3) dx = -1; // West

        int nx = cx + dx;
        int ny = cy + dy;

        GridData& gridData = manager->GetGridData();
        GridCell* neighbor = gridData.GetCell(nx, ny);
        if (!neighbor) return;

        // Identify the first active material payload currently residing on the conveyor belt
        int targetItemType = -1;
        for (auto const& [key, val] : cell->data) {
            if (key.rfind("i::", 0) == 0 && val > 0.0f) {
                targetItemType = std::stoi(key.substr(3));
                break;
            }
        }

        // If an item payload is found, broadcast a programmatic deposit payload down the grid path
        if (targetItemType != -1) {
            CellMessage depositMsg;
            depositMsg.type = "deposit";
            depositMsg.data["item_type"] = static_cast<float>(targetItemType);
            depositMsg.data["amount"] = 1.0f; // Standard transfer speed rating per system tick

            int acceptedAmount = manager->SendMessageToCell(nx, ny, depositMsg);
            if (acceptedAmount > 0) {
                // Safely clear out our local state inventory to match the successful transfer
                Withdraw(cell, targetItemType, acceptedAmount);
            }
        }
    }

    int Conveyor::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Process directional updates pushed directly from editor interactions
        if (message.type == "set_direction") {
            auto dirIt = message.data.find("dir");
            if (dirIt != message.data.end()) {
                float requestedDir = dirIt->second;

                // Wrap rotation indices between valid 0-3 boundaries safely
                if (requestedDir < 0.0f) requestedDir = 3.0f;
                if (requestedDir > 3.0f) requestedDir = 0.0f;

                cell->data["direction"] = requestedDir;
                return 1;
            }
        }

        // Allow programmatic storage routing commands from other belts/inputs to pass right through
        return StorageBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> Conveyor::GetMessageList() const {
        // Grab default structural actions (such as Destroy) from the base chain
        std::vector<CellMessage> messages = StorageBehavior::GetMessageList();

        // Inject directional configuration entries for the editor layout UI context panel
        messages.push_back({ .label = "Face Direction: North", .type = "set_direction", .data = { {"dir", 0.0f} } });
        messages.push_back({ .label = "Face Direction: East",  .type = "set_direction", .data = { {"dir", 1.0f} } });
        messages.push_back({ .label = "Face Direction: South", .type = "set_direction", .data = { {"dir", 2.0f} } });
        messages.push_back({ .label = "Face Direction: West",  .type = "set_direction", .data = { {"dir", 3.0f} } });

        return messages;
    }
}