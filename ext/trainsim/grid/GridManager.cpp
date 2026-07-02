#include "GridManager.h"
#include "GridManager.h"
#include <algorithm>
#include <cmath>

namespace Trainsim {
    GridManager* GridManager::s_instance = nullptr;

    GridManager::GridManager() {
        s_instance = this;
        InitializeTypeCache(); // Initial population of all default/empty tiles
    }

    // Include if utilizing custom initialization data injection
    GridManager::GridManager(const GridData& existingData) : gridData(existingData) {
        s_instance = this;
        InitializeTypeCache();
    }

    GridManager::~GridManager() {
        if (s_instance == this) s_instance = nullptr;
    }

    void GridManager::InitializeTypeCache() {
        typeCache.clear();
        for (int y = 0; y < gridData.height; ++y) {
            for (int x = 0; x < gridData.width; ++x) {
                GridCell* cell = gridData.GetCell(x, y);
                if (cell) {
                    typeCache[cell->type].push_back(cell);
                }
            }
        }
    }

    void GridManager::UpdateCell(GridCell* cell, const std::string& oldType) {
        if (!cell) return;
        if (oldType == cell->type) return; // Safeguard: type didn't actually mutate

        // 1. Remove the cell pointer from its historical category bucket
        auto it = typeCache.find(oldType);
        if (it != typeCache.end()) {
            auto& vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), cell), vec.end());
        }

        // 2. Insert into the newly assigned type classification vector
        typeCache[cell->type].push_back(cell);
    }

    const std::vector<GridCell*>& GridManager::GetCellsOfType(const std::string& type) const {
        static const std::vector<GridCell*> emptyVec;
        auto it = typeCache.find(type);
        if (it != typeCache.end()) {
            return it->second;
        }
        return emptyVec;
    }

    bool GridManager::FindCellCoordinates(const GridCell* cell, int& outX, int& outY) const {
        for (int y = 0; y < gridData.height; ++y) {
            for (int x = 0; x < gridData.width; ++x) {
                if (gridData.GetCell(x, y) == cell) {
                    outX = x;
                    outY = y;
                    return true;
                }
            }
        }
        return false;
    }

    std::pair<int, int> GridManager::WorldToGrid(float world_x, float world_y) const {
        int x = static_cast<int>(std::floor(world_x / cellSize));
        int y = static_cast<int>(std::floor(world_y / cellSize));
        return { x, y };
    }

    void GridManager::Select(float world_x, float world_y) {
        selectedCells.clear();
        auto [x, y] = WorldToGrid(world_x, world_y);

        if (x >= 0 && x < gridData.width && y >= 0 && y < gridData.height) {
            selectedCells.push_back({ x, y });
            anchorCell = { x, y }; // Anchor is set here for future BoxSelect calls
        }
        else {
            anchorCell = { -1, -1 };
        }
    }

    void GridManager::BoxSelect(float world_x, float world_y) {
        // If no valid anchor exists yet, default to a standard single selection
        if (anchorCell.first == -1 || anchorCell.second == -1) {
            Select(world_x, world_y);
            return;
        }

        auto [targetX, targetY] = WorldToGrid(world_x, world_y);

        // Bounds clamping ensures selection doesn't spill out of the valid grid space
        targetX = std::clamp(targetX, 0, gridData.width - 1);
        targetY = std::clamp(targetY, 0, gridData.height - 1);

        selectedCells.clear();

        int minX = std::min(anchorCell.first, targetX);
        int maxX = std::max(anchorCell.first, targetX);
        int minY = std::min(anchorCell.second, targetY);
        int maxY = std::max(anchorCell.second, targetY);

        // Populate selection with all coordinates spanning the bounding box
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                selectedCells.push_back({ x, y });
            }
        }
    }

    void GridManager::RegisterBehavior(const std::string& celltype, CellBehavior* behavior) {
        behaviors[celltype] = behavior;
    }
    // Returns true if at least one selected cell successfully processed the command
    bool GridManager::SendMessageToSelected(const CellMessage& message) {
        bool anyAccepted = false;
        for (auto const& [x, y] : selectedCells) {
            GridCell* cell = gridData.GetCell(x, y);
            if (!cell) continue;

            auto it = behaviors.find(cell->type);
            if (it != behaviors.end() && it->second != nullptr) {
                if (it->second->ReceiveMessage(cell, message)) {
                    anyAccepted = true;
                }
            }
        }
        return anyAccepted;
    }

    // Direct interface for programmatic target interactions
    int GridManager::SendMessageToCell(int x, int y, const CellMessage& message) {
        GridCell* cell = gridData.GetCell(x, y);
        if (!cell) return 0;

        auto it = behaviors.find(cell->type);
        if (it != behaviors.end() && it->second != nullptr) {
            return it->second->ReceiveMessage(cell, message);
        }
        return 0;
    }

    void GridManager::Update(float deltatime) {
		timeLastTicked += deltatime;
        if (timeLastTicked < tickDuration)
            return;
        else
			timeLastTicked = 0.0f;

        for (int y = 0; y < gridData.height; ++y) {
            for (int x = 0; x < gridData.width; ++x) {
                GridCell* cell = gridData.GetCell(x, y);
                if (cell) {
                    auto it = behaviors.find(cell->type);
                    if (it != behaviors.end() && it->second != nullptr) {
                        it->second->Update(cell);
                    }
                }
            }
        }
    }

    std::vector<CellMessage> GridManager::GetCompatibleMessages(const std::string& celltype) const {
        auto it = behaviors.find(celltype);
        if (it != behaviors.end() && it->second != nullptr) {
            return it->second->GetMessageList();
        }
        return {};
    }
    CellBehavior* Trainsim::GridManager::GetBehavior(const std::string& type) const
    {
        auto it = behaviors.find(type);
        if (it != behaviors.end()) return it->second;
        return nullptr;
    }
}