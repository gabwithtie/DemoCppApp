#pragma once

#include "train/TrainGameModels.hpp"
#include "finance/FinanceManager.hpp"
#include <vector>
#include <unordered_map>

namespace app {

    class GameSimulation {
    public:
        GameSimulation();

        void InitializeSampleData();
        void Update(float deltaTime);

        bool SetTrainDestination(const std::string& train_id, const std::string& target_station_id);
        std::vector<std::string> FindPath(const std::string& start_id, const std::string& end_id) const;

        // Cargo Purchase & Sale Logic
        bool BuyCargoFromStation(const std::string& station_id, const std::string& train_id, size_t carriage_idx, const std::string& item_id, int quantity);
        bool SellCargoToStation(const std::string& train_id, size_t carriage_idx, const std::string& station_id, const std::string& item_id, int quantity);
        double GetItemSellPriceAtStation(const Station& station, const std::string& item_id) const;

        void SelectStation(const std::string& station_id) { m_selected_station_id = station_id; }
        const std::string& GetSelectedStationId() const { return m_selected_station_id; }

        void SelectTrain(const std::string& train_id) { m_selected_train_id = train_id; }
        const std::string& GetSelectedTrainId() const { return m_selected_train_id; }

        const std::vector<Station>& GetStations() const { return m_stations; }
        std::vector<Station>& GetStations() { return m_stations; }
        const std::vector<TrackLine>& GetLines() const { return m_lines; }
        std::vector<Train>& GetTrains() { return m_trains; }
        const std::vector<Train>& GetTrains() const { return m_trains; }
        const FinanceManager& GetFinanceManager() const { return m_finance; }
        FinanceManager& GetFinanceManager() { return m_finance; }

        const Station* FindStation(const std::string& id) const;
        Station* FindStationMutable(const std::string& id);
        const Train* FindTrain(const std::string& id) const;
        Train* FindTrainMutable(const std::string& id);

        gbe::Vector2 GetTrainWorldPosition(const Train& train) const;

    private:
        std::vector<Station> m_stations;
        std::vector<TrackLine> m_lines;
        std::vector<Train> m_trains;
        std::vector<CargoTypeDefinition> m_cargo_definitions;

        FinanceManager m_finance{ 100000.0 };
        std::string m_selected_station_id;
        std::string m_selected_train_id;

        std::unordered_map<std::string, size_t> m_station_map;
        std::unordered_map<std::string, std::vector<std::string>> m_graph;

        void RebuildGraph();
        void RegenerateStationMarket(Station& station);
    };

} // namespace app