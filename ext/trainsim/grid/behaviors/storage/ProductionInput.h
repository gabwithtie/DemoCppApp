#pragma once
#include "StorageBehavior.h"

namespace Trainsim {
    class ProductionInput : public StorageBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;
    };
}