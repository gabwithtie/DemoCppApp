#pragma once
#include "trainsim/grid/behaviors/BuildingBehavior.h"
#include <string>
#include <vector>
#include <utility>

namespace Trainsim {
    class LineBehavior : public BuildingBehavior {
    public:
        virtual void Update(GridCell* cell) override;
        virtual int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        virtual std::vector<CellMessage> GetMessageList() const override;

    protected:
        // Child classes must define their connection rules
        virtual float GetMaxRadius(const GridCell* cell) const = 0;
        virtual std::string GetTargetBuildingType() const = 0;

        // Optional limit for connections. Return -1 for unlimited.
        virtual int GetMaxConnections() const { return -1; }
    };
}