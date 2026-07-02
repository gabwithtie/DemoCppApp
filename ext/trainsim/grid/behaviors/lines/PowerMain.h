#pragma once
#include "LineBehavior.h"

namespace Trainsim {
    class PowerMain : public LineBehavior {
    protected:
        float GetMaxRadius(const GridCell* cell) const override;
        std::string GetTargetBuildingType() const override;
    };
}