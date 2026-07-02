#include "BuilderHolder.h"
#include "trainsim/grid/GridManager.h"
#include <cmath>
#include <algorithm>

namespace Trainsim {
    enum BuilderState {
        STATE_IDLE = 0,
        STATE_WAITING_FOR_RESOURCES,
        STATE_MOVING_TO_TARGET,
        STATE_RETURNING
    };

    void BuilderHolder::Update(GridCell* cell) {
        if (!cell) return;

        // 1. Ensure entity base fields are initialized
        EntityHolder::Update(cell);

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        // Initialize state machine tracker if completely missing
        if (cell->data.find("builder_state") == cell->data.end()) {
            cell->data["builder_state"] = static_cast<float>(STATE_IDLE);
        }

        BuilderState state = static_cast<BuilderState>(cell->data["builder_state"]);
        GridData& gridData = manager->GetGridData();

        // -------------------------------------------------------------
        // STATE 0: IDLE (Scan grid for damaged/unbuilt structures)
        // -------------------------------------------------------------
        if (state == STATE_IDLE) {
            GridCell* targetCell = nullptr;

            // Start at 1 to skip BuildingType::EMPTY
            int start = static_cast<int>(BuildingType::EMPTY) + 1;
            int end = static_cast<int>(BuildingType::MAX);

            for (int i = start; i < end; ++i) {
                std::string typeStr = std::string(magic_enum::enum_name(static_cast<BuildingType>(i)));

                // Fetch the pre-cached list of cells for this specific building type
                const auto& cachedCells = manager->GetCellsOfType(typeStr);

                for (GridCell* c : cachedCells) {
                    if (c) {
                        if (c->data.find("health") == c->data.end()) {
                            c->data["health"] = (typeStr == "HEADQUARTERS") ? 1.0f : 0.0f;
                        }

                        if (c->data["health"] < 1.0f) {
                            targetCell = c;
                            break;
                        }
                    }
                }

                if (targetCell) break; // Break outer loop if a target was found
            }

            int tx = 0, ty = 0;
            // Only pay the cost to find coordinates once we actually have a target
            if (targetCell && manager->FindCellCoordinates(targetCell, tx, ty)) {
                cell->data["target_x"] = static_cast<float>(tx);
                cell->data["target_y"] = static_cast<float>(ty);
                cell->data["builder_state"] = static_cast<float>(STATE_WAITING_FOR_RESOURCES);
            }
        }

        // -------------------------------------------------------------
        // STATE 1: WAITING FOR RESOURCES (Pool adjacent storage containers)
        // -------------------------------------------------------------
        else if (state == STATE_WAITING_FOR_RESOURCES) {
            int tx = static_cast<int>(cell->data["target_x"]);
            int ty = static_cast<int>(cell->data["target_y"]);
            GridCell* targetCell = gridData.GetCell(tx, ty);

            // Safety fallback: Target was destroyed or repaired while waiting
            if (!targetCell || targetCell->type == "EMPTY" || targetCell->data["health"] >= 1.0f) {
                cell->data["builder_state"] = static_cast<float>(STATE_IDLE);
                return;
            }

            // Extract the custom building cost requirement vector
            auto rawBehavior = manager->GetBehavior(targetCell->type);
            auto bBehavior = dynamic_cast<BuildingBehavior*>(rawBehavior);
            std::map<int, int> buildCost = bBehavior ? bBehavior->GetBuildCost() : std::map<int, int>{ {1, 5} };

            // Check the 4 adjacent neighbors around this BuilderHolder
            const int dx[] = { 1, -1, 0, 0 };
            const int dy[] = { 0, 0, 1, -1 };

            bool neighborsHaveEnough = true;

            // Step A: Verification Pass (Check aggregate availability)
            for (auto const& [itemType, amountNeeded] : buildCost) {
                int availablePool = 0;
                std::string itemKey = "i::" + std::to_string(itemType);

                for (int i = 0; i < 4; ++i) {
                    GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
                    if (neighbor && neighbor->data.find(itemKey) != neighbor->data.end()) {
                        availablePool += static_cast<int>(neighbor->data[itemKey]);
                    }
                }

                if (availablePool < amountNeeded) {
                    neighborsHaveEnough = false;
                    break; // Still short on materials, wait here
                }
            }

            // Step B: Withdrawal Pass (Consume materials if aggregate totals are satisfied)
            if (neighborsHaveEnough) {
                for (auto const& [itemType, amountNeeded] : buildCost) {
                    int remainingToDeduct = amountNeeded;
                    std::string itemKey = "i::" + std::to_string(itemType);

                    for (int i = 0; i < 4; ++i) {
                        if (remainingToDeduct <= 0) break;
                        GridCell* neighbor = gridData.GetCell(cx + dx[i], cy + dy[i]);
                        if (neighbor && neighbor->data.find(itemKey) != neighbor->data.end()) {
                            int available = static_cast<int>(neighbor->data[itemKey]);
                            int deducted = std::min(remainingToDeduct, available);
                            neighbor->data[itemKey] -= static_cast<float>(deducted);
                            remainingToDeduct -= deducted;
                        }
                    }
                }
                // Resources secured inside infinite builder inventory, begin layout pathing
                cell->data["builder_state"] = static_cast<float>(STATE_MOVING_TO_TARGET);
            }
        }

        // -------------------------------------------------------------
        // STATE 2: MOVING TO TARGET
        // -------------------------------------------------------------
        else if (state == STATE_MOVING_TO_TARGET) {
            float ex = cell->data["entity_x"];
            float ey = cell->data["entity_y"];
            float tx = cell->data["target_x"];
            float ty = cell->data["target_y"];

            float dx = tx - ex;
            float dy = ty - ey;
            float distance = std::sqrt(dx * dx + dy * dy);

            float movementSpeedPerTick = 0.5f; // Adjust to dial in step animation speed

            if (distance <= movementSpeedPerTick) {
                cell->data["entity_x"] = tx;
                cell->data["entity_y"] = ty;

                // Entity successfully touches the target grid frame cell
                GridCell* targetCell = gridData.GetCell(static_cast<int>(tx), static_cast<int>(ty));
                if (targetCell) {
                    targetCell->data["health"] = 1.0f; // Instant reconstruction completion
                }
                cell->data["builder_state"] = static_cast<float>(STATE_RETURNING);
            }
            else {
                cell->data["entity_x"] = ex + (dx / distance) * movementSpeedPerTick;
                cell->data["entity_y"] = ey + (dy / distance) * movementSpeedPerTick;
            }
        }

        // -------------------------------------------------------------
        // STATE 3: RETURNING TO BASE
        // -------------------------------------------------------------
        else if (state == STATE_RETURNING) {
            float ex = cell->data["entity_x"];
            float ey = cell->data["entity_y"];
            float hx = static_cast<float>(cx);
            float hy = static_cast<float>(cy);

            float dx = hx - ex;
            float dy = hy - ey;
            float distance = std::sqrt(dx * dx + dy * dy);

            float movementSpeedPerTick = 0.5f;

            if (distance <= movementSpeedPerTick) {
                cell->data["entity_x"] = hx;
                cell->data["entity_y"] = hy;
                cell->data["builder_state"] = static_cast<float>(STATE_IDLE); // Return to rest
            }
            else {
                cell->data["entity_x"] = ex + (dx / distance) * movementSpeedPerTick;
                cell->data["entity_y"] = ey + (dy / distance) * movementSpeedPerTick;
            }
        }
    }
}