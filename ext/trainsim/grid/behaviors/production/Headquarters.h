#pragma once
#include "ProductionBehavior.h"

namespace Trainsim {
    class Headquarters : public ProductionBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;

    protected:
        std::map<int, int> GetRecipeInputs(const GridCell* cell) const override;
        std::pair<int, int> GetRecipeOutput(const GridCell* cell) const override;

    private:
        // --- NEW OUTPOST LOGIC ---
        void ClaimTerritory(GridCell* cell);
        void ReleaseClaims(GridCell* cell);
    };
}