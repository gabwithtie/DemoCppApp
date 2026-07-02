// StorageBehavior.h
#pragma once
#include "trainsim/grid/behaviors/BuildingBehavior.h"

namespace Trainsim {
    class StorageBehavior : public BuildingBehavior {
        public:
        // Processes unified programmatic programmatic storage exchanges
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
    protected:
        // Returns total consolidated counts across all internal item channels
        int GetTotalStorageUsed(const GridCell* cell) const;

        // Returns 1 if successful, 0 if capacity limit constraints prevent deposit action
        int Deposit(GridCell* cell, int itemtype, int amount);

        // Removes up to requested value, returning quantity pulled
        int Withdraw(GridCell* cell, int itemtype, int amount);
    };
}