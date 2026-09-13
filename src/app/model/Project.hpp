#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "image/Layer.hpp"

namespace Model {

struct Project {
    std::string title{"Untitled Save"};

    int w = 2048;
    int h = 2048;

    std::vector<Layer> layers;
};

} // namespace Model