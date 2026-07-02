#pragma once
#include <vector>
#include "GridCell.h"
namespace Trainsim {
    class GridData {
    public:
        int width;
        int height;
        int seed = 1234;
        std::vector<GridCell> cells;

		std::unordered_map<std::string, float> globalData; // Global data storage for the entire grid

        // Default 100x100 constructor or custom dimensions
        GridData(int w = 100, int h = 100)
            : width(w), height(h), cells(w* h) {
        }

        // Safe accessor for retrieving a cell pointer via coordinates
        GridCell* GetCell(int x, int y) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                return &cells[y * width + x];
            }
            return nullptr;
        }

        const GridCell* GetCell(int x, int y) const {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                return &cells[y * width + x];
            }
            return nullptr;
        }
    };
}