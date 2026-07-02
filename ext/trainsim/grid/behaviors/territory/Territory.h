#pragma once
#include "trainsim/grid/common/CellBehavior.h"

namespace Trainsim {
    class Territory : public CellBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        std::vector<CellMessage> GetMessageList() const override;
    };
}