#include "ProductionOutput.h"
#include "trainsim/grid/GridManager.h"
#include <vector>

namespace Trainsim {
    void ProductionOutput::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("max_storage") == cell->data.end()) cell->data["max_storage"] = 100.0f;

        GridManager* manager = GridManager::Get();
		auto gridData = manager->GetGridData();

        if (!manager) return;

        DoOnAdjacent(cell, [&](int nx, int ny) {
            int response = manager->SendMessageToCell(nx, ny, { .type = "get_production_output" });
            if (response > 0) { // Valid production node yielding an item type
                int outputItemType = response;

                int limitCap = static_cast<int>(cell->data["max_storage"]);
                int remainingRoom = limitCap - GetTotalStorageUsed(cell);

                if (remainingRoom <= 0) return;

                CellMessage withdrawMsg;
                withdrawMsg.type = "withdraw";
                withdrawMsg.data["amount"] = static_cast<float>(remainingRoom);

                int itemsExtracted = manager->SendMessageToCell(nx, ny, withdrawMsg);
                if (itemsExtracted > 0) {
                    Deposit(cell, outputItemType, itemsExtracted); // Direct strict type sorting achieved!
                }
            }
            });

        DoOnAdjacent(cell, [&](int nx, int ny) {
            // If neighbor responds with any non-zero value, it is a production building. Avoid feeding it.
            if (gridData.GetCell(nx, ny)->type == magic_enum::enum_name(BuildingType::EMPTY) || manager->SendMessageToCell(nx, ny, { .type = "get_production_output" }) != 0) {
                return;
            }

            std::vector<int> availableItemTypes;
            for (auto const& [key, val] : cell->data) {
                if (key.rfind("i::", 0) == 0 && val > 0.0f) {
                    availableItemTypes.push_back(std::stoi(key.substr(3)));
                }
            }

            for (int itemType : availableItemTypes) {
                std::string key = "i::" + std::to_string(itemType);
                if (cell->data.find(key) == cell->data.end() || cell->data[key] <= 0.0f) continue;

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
            });
    }

    int ProductionOutput::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        return StorageBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> ProductionOutput::GetMessageList() const {
        return StorageBehavior::GetMessageList();
    }
    void ProductionOutput::PullOutputs(GridCell* center, ItemType type, float& stockpile, float needed)
    {
        DoOnAdjacent(center, [&](int nx, int ny) {
            GridManager* manager = GridManager::Get();

            int response = manager->SendMessageToCell(nx, ny, { .type = "get_production_output" });
            if (response == static_cast<int>(type)) { // Valid production node yielding an item type
                int outputItemType = response;

                int remainingRoom = needed - stockpile;

                if (remainingRoom <= 0) return;

                CellMessage withdrawMsg;
                withdrawMsg.type = "withdraw";
                withdrawMsg.data["amount"] = static_cast<float>(remainingRoom);

                int itemsExtracted = manager->SendMessageToCell(nx, ny, withdrawMsg);
                if (itemsExtracted > 0) {
                    stockpile += itemsExtracted; // Direct strict type sorting achieved!
                }
            }
			});

    }
}