#pragma once
#include <string>
#include <vector>
#include <cstdint>

#define FURNITURE_ID_EMPTY "FURNITUREIDEMPTY"
#define FURNITURE_ID_PASSENGERSET "FURNITUREIDPASSENGERSET"
#define FURNITURE_ID_CARGOSHELF "FURNITUREIDCARGOSHELF"

#define CARRIAGE_ID_PASSENGER "CARRIAGEIDPASSENGER"
#define CARRIAGE_ID_CARGO "CARRIAGEIDCARGO"
#define CARRIAGE_ID_HEAD "CARRIAGEIDHEAD"

namespace Model {

struct FurnitureSlot{
    std::string type; // FURNITURE_ID
};

struct Carriage{
    std::string type; // CARRIAGE_ID

    std::vector<FurnitureSlot> slots; // a carriage always has 8 slots: 4 left and 4 right.
};

struct Train {
    std::vector<Carriage> carriages;
};

} // namespace Model