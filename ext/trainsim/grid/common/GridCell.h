#pragma once
#include <string>
#include <unordered_map>

#include "trainsim/grid/BuildingTypes.h"
#include <magic_enum.hpp>

namespace Trainsim {
    struct GridCell {
        std::string type = std::string(magic_enum::enum_name(BuildingType::EMPTY));
        std::unordered_map<std::string, float> data;
    };
}