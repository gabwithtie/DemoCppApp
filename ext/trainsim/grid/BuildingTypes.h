#pragma once
#include <unordered_map>
#include <string>

namespace Trainsim {
    // Cross-reference registry mapping raw numerical IDs to cell types
    enum class GroundType {
        EMPTY = 0,
        TERRITORY,

        MAX
    };

    enum class BuildingType
    {
        EMPTY = 0,
        HEADQUARTERS,
        PRODUCTION_OUTPUT,
        PRODUCTION_INPUT,
        MINER,
        CONVEYOR,
        BUILDER,
        POWER_MAIN,
        POWER_TERMINAL,
        OUTPOST,

        MAX
    };


}