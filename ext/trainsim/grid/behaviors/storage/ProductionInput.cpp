#include "ProductionInput.h"
#include "trainsim/grid/GridManager.h"
#include <vector>

namespace Trainsim {
    void ProductionInput::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("max_storage") == cell->data.end()) cell->data["max_storage"] = 100.0f;

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        // Collect available internal item inventories to protect loop iterators
        std::vector<int> availableItemTypes;
        for (auto const& [key, val] : cell->data) {
            if (key.rfind("i::", 0) == 0 && val > 0.0f) {
                availableItemTypes.push_back(std::stoi(key.substr(3)));
            }
        }

        // PUSH PHASE: Scan neighbors and verify exact input compatibility before dispatching deposits
        DoOnAdjacent(cell, [&](int nx, int ny) {
            for (int itemType : availableItemTypes) {
                std::string key = "i::" + std::to_string(itemType);
                if (cell->data.find(key) == cell->data.end() || cell->data[key] <= 0.0f) continue;

                // Query strict input requirement matching
                CellMessage checkMsg;
                checkMsg.type = "is_production_input";
                checkMsg.data["item_type"] = static_cast<float>(itemType);

                if (manager->SendMessageToCell(nx, ny, checkMsg) == 1) {
                    int amountAvailable = static_cast<int>(cell->data[key]);

                    CellMessage depositMsg;
                    depositMsg.type = "deposit";
                    depositMsg.data["item_type"] = static_cast<float>(itemType);
                    depositMsg.data["amount"] = static_cast<float>(amountAvailable);

                    int itemsDeposited = manager->SendMessageToCell(nx, ny, depositMsg);
                    if (itemsDeposited > 0) {
                        Withdraw(cell, itemType, itemsDeposited);
                    }
                }
            }
            });
    }

    int ProductionInput::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        return StorageBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> ProductionInput::GetMessageList() const {
        return StorageBehavior::GetMessageList();
    }
}