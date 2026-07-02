#pragma once
#include "trainsim/grid/common/CellBehavior.h"

#include "trainsim/resource/ItemTypes.h"

#include <map>

namespace Trainsim {
    class BuildingBehavior : public CellBehavior {
    public:
        // Provide a default empty update so static buildings don't strictly have to implement it
        virtual void Update(GridCell* cell) override { (void)cell; }

        virtual int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        virtual std::vector<CellMessage> GetMessageList() const override;

        virtual std::map<int, int> GetBuildCost() const {
            // Default baseline fallback cost if a child class forgets to specify one
            return { { static_cast<int>(ItemType::STEEL), 5}}; // e.g., 5 STEEL
        }
    };
}