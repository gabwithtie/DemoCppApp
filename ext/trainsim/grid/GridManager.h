#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include "common/GridData.h"
#include "common/CellBehavior.h"
#include "common/CellMessage.h"

namespace Trainsim {
    class GridManager {
    private:
        GridData gridData = { 100, 100 };
        std::unordered_map<std::string, CellBehavior*> behaviors;

        // --- TYPE CACHE OPTIMIZATION ---
        // Maps a cell type string (e.g., "BASE", "FACTORY") to direct pointers for instant lookup
        std::unordered_map<std::string, std::vector<GridCell*>> typeCache;
        void InitializeTypeCache();

        std::vector<std::pair<int, int>> selectedCells;
        std::pair<int, int> anchorCell = { -1, -1 };
        float cellSize = 1.0f;

        std::pair<int, int> WorldToGrid(float world_x, float world_y) const;

        float tickDuration = 0.3f;
        float timeLastTicked = 0.0f;

        static GridManager* s_instance;
    public:
        GridManager();
        GridManager(const GridData& existingData); // If you have a matching constructor overload
        ~GridManager();

        static GridManager* Get() { return s_instance; }

        // --- NEW CACHE API ---
        // Call this immediately after mutating a cell's type field to sync the index buckets
        void UpdateCell(GridCell* cell, const std::string& oldType);

        // Instantly fetches all allocated cells matching a type profile
        const std::vector<GridCell*>& GetCellsOfType(const std::string& type) const;

        void Select(float world_x, float world_y);
        void BoxSelect(float world_x, float world_y);
        void RegisterBehavior(const std::string& celltype, CellBehavior* behavior);
        bool SendMessageToSelected(const CellMessage& message);
        int SendMessageToCell(int x, int y, const CellMessage& message);
        void Update(float deltatime);
        bool FindCellCoordinates(const GridCell* cell, int& outX, int& outY) const;

        GridData& GetGridData() { return gridData; }
        const std::vector<std::pair<int, int>>& GetSelectedCells() const { return selectedCells; }
        void SetCellSize(float size) { cellSize = size; }
        std::vector<CellMessage> GetCompatibleMessages(const std::string& celltype) const;

        CellBehavior* GetBehavior(const std::string& type) const;
    };
}