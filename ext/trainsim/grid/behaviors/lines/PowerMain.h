#pragma once
#include "LineBehavior.h"

namespace Trainsim {
    class PowerMain : public LineBehavior {
    public:
        void Update(GridCell* cell) override;
        int ReceiveMessage(GridCell* cell, const CellMessage& message) override;

    protected:
        float GetMaxRadius(const GridCell* cell) const override;
        std::string GetTargetBuildingType() const override;

    private:
        // Helper to ping a specific terminal with our connection status
        void NotifyTerminal(int tx, int ty, const std::string& messageType);
    };
}