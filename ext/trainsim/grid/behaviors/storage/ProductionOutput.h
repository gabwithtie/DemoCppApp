#pragma once
#include "StorageBehavior.h"

namespace Trainsim {
    class ProductionOutput : public StorageBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;

		static void PullOutputs(GridCell* center, ItemType type, float& stockpile, float needed);
    };
}