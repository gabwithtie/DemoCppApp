#pragma once

#include "map/MapManager.hpp"
#include "train/TrainManager.hpp"
#include "station/MarketManager.hpp"
#include "finance/FinanceManager.hpp"
#include <string>

namespace app {

    class GameSimulation {
    public:
        GameSimulation();

        void InitializeSampleData();
        void Update(float deltaTime);

        // Sub-module Accessors
        MapManager& GetMapManager() { return m_map; }
        const MapManager& GetMapManager() const { return m_map; }

        TrainManager& GetTrainManager() { return m_trains; }
        const TrainManager& GetTrainManager() const { return m_trains; }

        MarketManager& GetMarketManager() { return m_market; }
        const MarketManager& GetMarketManager() const { return m_market; }

        FinanceManager& GetFinanceManager() { return m_finance; }
        const FinanceManager& GetFinanceManager() const { return m_finance; }

        // UI / Selection State
        void SelectStation(const std::string& station_id) { m_selected_station_id = station_id; }
        const std::string& GetSelectedStationId() const { return m_selected_station_id; }

        void SelectTrain(const std::string& train_id) { m_selected_train_id = train_id; }
        const std::string& GetSelectedTrainId() const { return m_selected_train_id; }

    private:
        // Initialization order: dependencies must precede dependent managers
        MapManager m_map;
        FinanceManager m_finance{ 100000.0 };
        TrainManager m_trains;
        MarketManager m_market;

        std::string m_selected_station_id;
        std::string m_selected_train_id;
    };

} // namespace app