#include "CellBehavior.h"
#include "CellBehavior.h"

#include "trainsim/grid/GridManager.h"

namespace Trainsim {
	void CellBehavior::DoOnAdjacent(GridCell* center, std::function<void(int x, int y)> func)
    {
        if (!center) return;
        // Assuming you have a way to get the coordinates of the center cell
        int cx, cy;
        if (!GridManager::Get()->FindCellCoordinates(center, cx, cy)) return;
        // Iterate over adjacent cells (up, down, left, right)
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (abs(dx) + abs(dy) == 1) { // Only consider direct neighbors
                    int nx = cx + dx;
                    int ny = cy + dy;
                    func(nx, ny);
                }
            }
        }
    }
    void Trainsim::CellBehavior::DoOnRadius(GridCell* center, float radius, std::function<void(int nx, int ny)> func)
    {
        if (!center) return;
        int cx, cy;
        if (!GridManager::Get()->FindCellCoordinates(center, cx, cy)) return;
        int r = static_cast<int>(std::ceil(radius));
        for (int dx = -r; dx <= r; ++dx) {
            for (int dy = -r; dy <= r; ++dy) {
                if (std::sqrt(dx * dx + dy * dy) <= radius) {
                    int nx = cx + dx;
                    int ny = cy + dy;
                    func(nx, ny);
                }
            }
		}
    }
}