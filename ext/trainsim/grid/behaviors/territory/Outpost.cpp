#include "Outpost.h"
#include "trainsim/grid/GridManager.h"
#include "trainsim/grid/BuildingTypes.h"
#include <magic_enum.hpp>
#include <set>
#include <string>
#include <cmath>

namespace Trainsim {

    void Outpost::ReleaseClaims(GridCell* cell) {
        if (!cell) return;
        GridManager* manager = GridManager::Get();
        if (!manager) return;
        GridData& gridData = manager->GetGridData();

        int index = 0;
        while (true) {
            std::string keyX = "claim::" + std::to_string(index) + "::x";
            std::string keyY = "claim::" + std::to_string(index) + "::y";

            if (cell->data.find(keyX) == cell->data.end() || cell->data.find(keyY) == cell->data.end()) {
                break;
            }

            int tx = static_cast<int>(cell->data[keyX]);
            int ty = static_cast<int>(cell->data[keyY]);

            GridCell* target = gridData.GetCell(tx, ty);
            if (target && target->type == std::string(magic_enum::enum_name(GroundType::TERRITORY))) {
                target->data["claim_count"] -= 1.0f;

                // Only revert if we are the absolute last outpost claiming this tile
                if (target->data["claim_count"] <= 0.0f) {
                    std::string oldType = target->type;
                    target->type = std::string(magic_enum::enum_name(BuildingType::EMPTY));
                    target->data.erase("claim_count");
                    manager->UpdateCell(target, oldType);
                }
            }

            cell->data.erase(keyX);
            cell->data.erase(keyY);
            index++;
        }
    }

    void Outpost::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("health") == cell->data.end()) cell->data["health"] = 0.0f;

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        GridData& gridData = manager->GetGridData();

        // Check if fully built and successfully powered
        bool isActive = (cell->data["health"] >= 1.0f) &&
            (cell->data.find("powered") != cell->data.end() && cell->data["powered"] > 0.0f);

        // 1. Gather what we are CURRENTLY claiming
        std::set<std::pair<int, int>> currentClaims;
        int index = 0;
        while (true) {
            std::string keyX = "claim::" + std::to_string(index) + "::x";
            std::string keyY = "claim::" + std::to_string(index) + "::y";

            if (cell->data.find(keyX) == cell->data.end() || cell->data.find(keyY) == cell->data.end()) {
                break;
            }
            currentClaims.insert({ static_cast<int>(cell->data[keyX]), static_cast<int>(cell->data[keyY]) });

            // Temporarily erase them to prepare for a clean rewrite below
            cell->data.erase(keyX);
            cell->data.erase(keyY);
            index++;
        }

        // 2. Scan the grid to calculate what we SHOULD be claiming
        std::set<std::pair<int, int>> desiredClaims;

        if (isActive) {
            float radius = GetRadius();
            int radInt = static_cast<int>(std::ceil(radius));
            std::string emptyType = std::string(magic_enum::enum_name(BuildingType::EMPTY));
            std::string territoryType = std::string(magic_enum::enum_name(GroundType::TERRITORY));

            for (int dy = -radInt; dy <= radInt; ++dy) {
                for (int dx = -radInt; dx <= radInt; ++dx) {
                    if (dx == 0 && dy == 0) continue; // Skip self

                    if (std::sqrt(dx * dx + dy * dy) <= radius) {
                        int nx = cx + dx;
                        int ny = cy + dy;
                        GridCell* target = gridData.GetCell(nx, ny);

                        // Only attempt to claim it if it isn't occupied by a physical building
                        if (target && (target->type == emptyType || target->type == territoryType)) {
                            desiredClaims.insert({ nx, ny });
                        }
                    }
                }
            }
        }

        // 3. Process Removals (Cells we claimed last tick, but lost this tick - e.g. unpowered or user placed a building)
        for (const auto& pos : currentClaims) {
            if (desiredClaims.find(pos) == desiredClaims.end()) {
                GridCell* target = gridData.GetCell(pos.first, pos.second);
                if (target && target->type == std::string(magic_enum::enum_name(GroundType::TERRITORY))) {
                    target->data["claim_count"] -= 1.0f;

                    if (target->data["claim_count"] <= 0.0f) {
                        std::string oldType = target->type;
                        target->type = std::string(magic_enum::enum_name(BuildingType::EMPTY));
                        target->data.erase("claim_count");
                        manager->UpdateCell(target, oldType); // Alert caching system!
                    }
                }
            }
        }

        // 4. Process Additions (Cells we weren't claiming last tick, but are claiming now)
        for (const auto& pos : desiredClaims) {
            if (currentClaims.find(pos) == currentClaims.end()) {
                GridCell* target = gridData.GetCell(pos.first, pos.second);
                if (target) {
                    std::string emptyType = std::string(magic_enum::enum_name(BuildingType::EMPTY));
                    std::string territoryType = std::string(magic_enum::enum_name(GroundType::TERRITORY));

                    if (target->type == emptyType) {
                        std::string oldType = target->type;
                        target->type = territoryType;
                        target->data["claim_count"] = 1.0f;
                        manager->UpdateCell(target, oldType); // Alert caching system!
                    }
                    else if (target->type == territoryType) {
                        if (target->data.find("claim_count") == target->data.end()) {
                            target->data["claim_count"] = 0.0f; // Fix edge cases
                        }
                        target->data["claim_count"] += 1.0f;
                    }
                }
            }
        }

        // 5. Write verified active claims back into the cell's memory sequentially
        int outIndex = 0;
        for (const auto& pos : desiredClaims) {
            std::string keyX = "claim::" + std::to_string(outIndex) + "::x";
            std::string keyY = "claim::" + std::to_string(outIndex) + "::y";
            cell->data[keyX] = static_cast<float>(pos.first);
            cell->data[keyY] = static_cast<float>(pos.second);
            outIndex++;
        }
    }

    int Outpost::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Critical hook: If the user deletes the outpost with the tool, 
        // we must revoke all our territory references before the memory gets wiped!
        if (message.type == "destroy") {
            ReleaseClaims(cell);
        }

        return BuildingBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> Outpost::GetMessageList() const {
        return BuildingBehavior::GetMessageList();
    }
}