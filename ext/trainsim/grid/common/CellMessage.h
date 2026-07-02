#pragma once
#include <string>

namespace Trainsim {
    struct CellMessage {
        std::string label;
        std::string type;
        std::unordered_map<std::string, float> data; // Payload parameters
    };
}