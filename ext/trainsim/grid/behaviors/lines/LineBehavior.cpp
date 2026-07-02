#include "LineBehavior.h"
#include "trainsim/grid/GridManager.h"
#include <cmath>
#include <algorithm>
#include <set>

namespace Trainsim {

    void LineBehavior::Update(GridCell* cell) {
        if (!cell) return;

        // Standard building health check
        if (cell->data.find("health") == cell->data.end()) cell->data["health"] = 0.0f;
        if (cell->data["health"] < 1.0f) return;

        GridManager* manager = GridManager::Get();
        if (!manager) return;

        int cx = 0, cy = 0;
        if (!manager->FindCellCoordinates(cell, cx, cy)) return;

        GridData& gridData = manager->GetGridData();
        std::string targetType = GetTargetBuildingType();
        float maxRadius = GetMaxRadius(cell);
        int maxConnections = GetMaxConnections();

        // 1. Extract and validate existing connections, then wipe the old keys
        std::vector<std::pair<int, int>> validConnections;
        std::set<std::pair<int, int>> connectionSet; // Used for fast duplicate checking

        int index = 0;
        while (true) {
            std::string keyX = "c::" + std::to_string(index) + "::x";
            std::string keyY = "c::" + std::to_string(index) + "::y";

            if (cell->data.find(keyX) == cell->data.end() || cell->data.find(keyY) == cell->data.end()) {
                break; // No more sequential connections found
            }

            int tx = static_cast<int>(cell->data[keyX]);
            int ty = static_cast<int>(cell->data[keyY]);

            // Validate the connection still exists and is the correct type
            GridCell* target = gridData.GetCell(tx, ty);
            if (target && target->type == targetType && target->data["health"] >= 1.0f) {
                float dx = static_cast<float>(tx - cx);
                float dy = static_cast<float>(ty - cy);
                if (std::sqrt(dx * dx + dy * dy) <= maxRadius) {
                    validConnections.push_back({ tx, ty });
                    connectionSet.insert({ tx, ty });
                }
            }

            // Erase the old dictionary entries to prep for a clean rewrite
            cell->data.erase(keyX);
            cell->data.erase(keyY);
            index++;
        }

        // 2. Scan for new connections if we haven't hit the capacity limit
        if (maxConnections == -1 || validConnections.size() < static_cast<size_t>(maxConnections)) {
            int radiusInt = static_cast<int>(std::ceil(maxRadius));

            for (int dy = -radiusInt; dy <= radiusInt; ++dy) {
                for (int dx = -radiusInt; dx <= radiusInt; ++dx) {
                    // Skip self
                    if (dx == 0 && dy == 0) continue;

                    if (std::sqrt(dx * dx + dy * dy) <= maxRadius) {
                        int nx = cx + dx;
                        int ny = cy + dy;

                        if (connectionSet.find({ nx, ny }) != connectionSet.end()) continue; // Already connected

                        GridCell* neighbor = gridData.GetCell(nx, ny);
                        if (neighbor && neighbor->type == targetType && neighbor->data["health"] >= 1.0f) {

                            validConnections.push_back({ nx, ny });
                            connectionSet.insert({ nx, ny });

                            // Stop if we hit the capacity limit
                            if (maxConnections != -1 && validConnections.size() >= static_cast<size_t>(maxConnections)) {
                                break;
                            }
                        }
                    }
                }
                if (maxConnections != -1 && validConnections.size() >= static_cast<size_t>(maxConnections)) break;
            }
        }

        // 3. Write the validated/new connections back into the cell data sequentially
        for (size_t i = 0; i < validConnections.size(); ++i) {
            std::string keyX = "c::" + std::to_string(i) + "::x";
            std::string keyY = "c::" + std::to_string(i) + "::y";
            cell->data[keyX] = static_cast<float>(validConnections[i].first);
            cell->data[keyY] = static_cast<float>(validConnections[i].second);
        }
    }

    int LineBehavior::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        return BuildingBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> LineBehavior::GetMessageList() const {
        return BuildingBehavior::GetMessageList();
    }
}