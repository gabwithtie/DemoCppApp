#include "Headquarters.h"
#include "trainsim/grid/GridManager.h"
#include "trainsim/resource/ItemTypes.h"
#include "trainsim/grid/BuildingTypes.h"
#include <magic_enum.hpp>
#include <cmath>
#include <string>

namespace Trainsim {

    void Headquarters::ClaimTerritory(GridCell* cell) {
        if (!cell) return;
        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        int radius = 5; // Adjust this to match your desired HQ territory size
        int claimIndex = 0;
        GridData& gridData = manager->GetGridData();

        // Scan the radius around the HQ
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                // Manhattan distance creates a diamond territory shape
                if (std::abs(x - cx) + std::abs(y - cy) <= radius) {
                    GridCell* target = gridData.GetCell(x, y);

                    // Only claim EMPTY cells or reinforce existing TERRITORY cells
                    if (target && target != cell && (target->type == "EMPTY" || target->type == "TERRITORY")) {

                        std::string oldType = target->type;
                        target->type = "TERRITORY";

                        if (target->data.find("claim_count") == target->data.end()) {
                            target->data["claim_count"] = 0.0f;
                        }
                        target->data["claim_count"] += 1.0f;

                        // If the cell type actually changed, sync the manager's cache
                        if (oldType != target->type) {
                            manager->UpdateCell(target, oldType);
                        }

                        // Store the coordinates in the HQ's memory
                        cell->data["claim::" + std::to_string(claimIndex) + "::x"] = static_cast<float>(x);
                        cell->data["claim::" + std::to_string(claimIndex) + "::y"] = static_cast<float>(y);
                        claimIndex++;
                    }
                }
            }
        }
    }

    void Headquarters::ReleaseClaims(GridCell* cell) {
        if (!cell) return;
        GridManager* manager = GridManager::Get();
        if (!manager) return;

        GridData& gridData = manager->GetGridData();
        int index = 0;

        // Iterate through all cached claims and revert them
        while (true) {
            std::string keyX = "claim::" + std::to_string(index) + "::x";
            std::string keyY = "claim::" + std::to_string(index) + "::y";

            // Break when we run out of indexed claims
            if (cell->data.find(keyX) == cell->data.end()) break;

            int tx = static_cast<int>(cell->data[keyX]);
            int ty = static_cast<int>(cell->data[keyY]);

            GridCell* target = gridData.GetCell(tx, ty);
            if (target && target->type == "TERRITORY") {
                target->data["claim_count"] -= 1.0f;

                // Once no nearby buildings are claiming this tile, return it to EMPTY
                if (target->data["claim_count"] <= 0.0f) {
                    std::string oldType = target->type;
                    target->type = std::string(magic_enum::enum_name(BuildingType::EMPTY));
                    target->data.erase("claim_count");

                    // Sync the manager's cache
                    manager->UpdateCell(target, oldType);
                }
            }

            // Wipe from memory
            cell->data.erase(keyX);
            cell->data.erase(keyY);
            index++;
        }
    }

    void Headquarters::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("health") == cell->data.end()) {
            cell->data["health"] = 1.0f;
        }
        if (cell->data.find("net_power") == cell->data.end()) {
            cell->data["net_power"] = 10.0f;
        }

        // 1. Singleton Constraint & Territory Initialization
        if (cell->data.find("hq_initialized") == cell->data.end()) {
            GridManager* manager = GridManager::Get();
            if (manager) {
                const auto& hqCells = manager->GetCellsOfType(cell->type);

                if (hqCells.size() > 1) {
                    CellMessage destroyMsg;
                    destroyMsg.type = "destroy";
                    ReceiveMessage(cell, destroyMsg);
                    return;
                }
            }

            // Flag as initialized so we don't run the expensive grid scan again
            cell->data["hq_initialized"] = 1.0f;

            // --- TRIGGER OUTPOST BEHAVIOR ---
            ClaimTerritory(cell);
        }

        DoOnAdjacent(cell, [](int nx, int ny) {
            auto neighbor = GridManager::Get()->GetGridData().GetCell(nx, ny);
            if (!neighbor) return;

            neighbor->data["health"] = 1.0f; // Ensure adjacent cells are fully operational
            });

        // 2. Execute standard production tick logic
        ProductionBehavior::Update(cell);
    }

    int Headquarters::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // --- INTERCEPT DESTRUCTION TO CLEAN UP TERRITORY ---
        if (message.type == "destroy") {
            ReleaseClaims(cell);
        }

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
        // Make sure to replace with your ItemTypes mapping if needed!
        return { static_cast<int>(ItemType::STEEL), 1};
    }
}