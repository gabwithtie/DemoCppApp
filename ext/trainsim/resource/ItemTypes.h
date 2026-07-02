#pragma once
#include <unordered_map>
#include <string>

namespace Trainsim {
    // Cross-reference registry mapping raw numerical IDs to cell types
    enum class ItemType
    {
        //MINED
        START_MINED = 0,

        IRON,
        COAL,
        BAUXITE,
        COPPER,
        QUARTZ,
        GOLD,

        END_MINED,

        //REFINED
        START_REFINED,

        STEEL,
        ALUMINUM,
        SILICON,

        END_REFINED,

        MAX
    };
}