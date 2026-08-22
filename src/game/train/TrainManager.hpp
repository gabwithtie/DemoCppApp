#pragma once

#include "TrainGameModels.hpp"
#include <vector>
#include <string>

namespace app {

    class MapManager;
    class FinanceManager;

    class TrainManager {
    public:
        TrainManager(MapManager& map, FinanceManager& finance);

        void InitializeSampleData();
        void Update(float deltaTime);

        bool SetTrainDestination(const std::string& train_id, const std::string& target_station_id);
        gbe::Vector2 GetTrainWorldPosition(const Train& train) const;

        const Train* FindTrain(const std::string& id) const;
        Train* FindTrainMutable(const std::string& id);

        std::vector<Train>& GetTrains() { return m_trains; }
        const std::vector<Train>& GetTrains() const { return m_trains; }

    private:
        MapManager& m_map;
        FinanceManager& m_finance;
        std::vector<Train> m_trains;
    };

} // namespace app