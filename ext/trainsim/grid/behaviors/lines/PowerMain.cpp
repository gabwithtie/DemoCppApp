#include "PowerMain.h"
#include "trainsim/grid/BuildingTypes.h"
#include <magic_enum.hpp>

namespace Trainsim {
    float PowerMain::GetMaxRadius(const GridCell* cell) const {
        return 10.0f; // Maximum distance to search for a terminal to connect to
    }

    std::string PowerMain::GetTargetBuildingType() const {
        // Automatically target the POWER_TERMINAL building type string
        return std::string(magic_enum::enum_name(BuildingType::POWER_TERMINAL));
    }
}