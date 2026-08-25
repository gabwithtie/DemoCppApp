#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "Train.hpp"
#include "Station.hpp"

namespace Model {

struct Project {
    std::string title{"Untitled Save"};

    Train train;
    std::vector<Station> stations;
    std::vector<StationConnection> station_connections;
};

} // namespace Model