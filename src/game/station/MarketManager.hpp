#pragma once

#include "TrainGameModels.hpp"
#include <vector>
#include <string>

namespace app {

    class MapManager;
    class TrainManager;
    class FinanceManager;

    class MarketManager {
    public:
        MarketManager(MapManager& map, TrainManager& trains, FinanceManager& finance);

        void RegenerateStationMarket(Station& station);
        double GetItemSellPriceAtStation(const Station& station, const std::string& item_id) const;

        bool BuyCargoFromStation(const std::string& station_id, const std::string& train_id, 
                                 size_t carriage_idx, const std::string& item_id, int quantity);
        
        bool SellCargoToStation(const std::string& train_id, size_t carriage_idx, 
                                const std::string& station_id, const std::string& item_id, int quantity);

    private:
        MapManager& m_map;
        TrainManager& m_trains;
        FinanceManager& m_finance;
        std::vector<CargoTypeDefinition> m_cargo_definitions;
    };

} // namespace app