#pragma once
#include "EntityHolder.h"

namespace Trainsim {
    class BuilderHolder : public EntityHolder {
    public:
        void Update(GridCell* cell) override;
    };
}