#pragma once
#include "trainsim/grid/behaviors/BuildingBehavior.h"

namespace Trainsim {
    class EntityHolder : public BuildingBehavior {
    public:
        void Update(GridCell* cell) override;
    };
}