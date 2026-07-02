#include "Headquarters.h"
#include "trainsim/grid/GridManager.h"
#include "trainsim/resource/ItemTypes.h"

namespace Trainsim {

    void Headquarters::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("health") == cell->data.end()) {
            cell->data["health"] = 1.0f;
        }
        if (cell->data.find("net_power") == cell->data.end()) {
            cell->data["net_power"] = 10.0f;
        }

        // 1. Singleton Constraint: Ensure only one Headquarters exists on the entire grid
        if (cell->data.find("hq_initialized") == cell->data.end()) {
            GridManager* manager = GridManager::Get();
            if (manager) {
                const auto& hqCells = GridManager::Get()->GetCellsOfType(cell->type);

                if (hqCells.size() > 1) {
                    CellMessage destroyMsg;
                    destroyMsg.type = "destroy";
                    ReceiveMessage(cell, destroyMsg);
                    return;
                }
            }

            // Flag as initialized so we don't run the expensive grid scan again
            cell->data["hq_initialized"] = 1.0f;
        }

        DoOnAdjacent(cell, [](int nx, int ny) {
			auto neighbor = GridManager::Get()->GetGridData().GetCell(nx, ny);

            if (!neighbor) return;
            
			neighbor->data["health"] = 1.0f; // Ensure adjacent cells are fully operational

			});

        DoOnRadius(cell, 5, [=](int nx, int ny) {
            auto neighbor = GridManager::Get()->GetGridData().GetCell(nx, ny);

			if (neighbor->type != std::string(magic_enum::enum_name(BuildingType::EMPTY))) return; // Skip non empty

			auto oldType = neighbor->type; // Cache the old type for GridManager update

            neighbor->type = std::string(magic_enum::enum_name(GroundType::TERRITORY));
            neighbor->data.clear(); // Wipe out default null state variables
            neighbor->data["claim_count"] = 1.0; // Default to 1 claimant

            GridManager::Get()->UpdateCell(neighbor, oldType);
            });

        // 2. Execute standard production tick logic
        ProductionBehavior::Update(cell);
    }

    int Headquarters::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        // Fall back to universal behavior handling (processes is_production, withdraw, destroy, etc.)
        return ProductionBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> Headquarters::GetMessageList() const {
        // Fall back to default actions (like Destroy Structure)
        return ProductionBehavior::GetMessageList();
    }

    std::map<int, int> Headquarters::GetRecipeInputs(const GridCell* cell) const {
        // Headquarters generates resources from nothing, so it requires no inputs
        return {};
    }

    std::pair<int, int> Headquarters::GetRecipeOutput(const GridCell* cell) const {
        // Singular output: solely provides STEEL to kickstart the player's production chain
        return { static_cast<int>(ItemType::STEEL), 1 };
    }
}