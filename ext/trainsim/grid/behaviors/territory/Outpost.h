#pragma once
#include "trainsim/grid/behaviors/BuildingBehavior.h"
#include <vector>
#include <utility>

namespace Trainsim {
    class Outpost : public BuildingBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;

    protected:
        virtual float GetRadius() const { return 8.0f; } // Area of influence

    private:
        // Helper to safely revoke this outpost's claims before being destroyed
        void ReleaseClaims(GridCell* cell);
    };
}