#pragma once
#include "ProductionBehavior.h"

namespace Trainsim {
    class Miner : public ProductionBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;

    protected:
        std::map<int, int> GetRecipeInputs(const GridCell* cell) const override;
        std::pair<int, int> GetRecipeOutput(const GridCell* cell) const override;

    private:
        // Core utility to wipe inventory and recalculate speed configurations
        void SetTargetItem(GridCell* cell, int itemType);

        // Pseudo-random hash generator for deterministic per-tile speeds
        int CalculateMiningSpeed(int x, int y, int seed, int itemType) const;
    };
}