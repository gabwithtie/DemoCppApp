#pragma once

#include "math/gbe_math.h"
#include <string>
#include <vector>
#include <cstdint>

namespace app {

    struct CargoDragPayload {
        char station_id[32];
        char item_id[64];
        double buy_price = 0.0;
        int available_quantity = 0;
    };

    struct CargoSellDragPayload {
        char train_id[32];
        size_t carriage_index = 0;
        char item_id[64];
        int available_quantity = 0;
    };

    struct MarketItem {
        std::string item_id;
        int quantity = 0;
        double buy_price = 0.0;
        double sell_price = 0.0;
    };

    struct Station {
        std::string id;
        std::string name;
        gbe::Vector2 position;

        std::vector<MarketItem> market;
        float regen_timer = 0.0f;
        float regen_interval = 8.0f;
    };

    struct TrackLine {
        std::string id;
        std::string name;
        uint32_t color = 0xFFFFFFFF;
        std::vector<std::string> station_ids;
    };

    struct InventoryItem {
        std::string item_id;
        int quantity = 0;
    };

    struct Carriage {
        std::string id;
        std::vector<InventoryItem> inventory;
    };

    struct Train {
        std::string id;
        float speed = 100.0f;

        std::string current_station_id;
        std::string target_station_id;

        std::vector<std::string> active_path;
        int current_segment = 0;
        float t = 0.0f;

        std::vector<Carriage> carriages;
    };

    struct CargoTypeDefinition {
        std::string id;
        std::string name;
        double base_price = 10.0;
    };

} // namespace app