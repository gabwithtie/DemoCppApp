#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "Items.hpp"
#include "Passenger.hpp"

namespace Model {
    struct Station{
        std::string id = ""; //STATION_ID

        std::vector<ItemStack> items;
        std::vector<Passenger> passengers;

        //Add future station meta data here
    };

    struct StationConnection {
        std::string from;
        std::string to;
    };
}