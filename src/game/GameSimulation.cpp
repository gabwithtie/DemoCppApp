#include "GameSimulation.hpp"

namespace app {

    GameSimulation::GameSimulation()
        : m_map(),
          m_finance(100000.0),
          m_trains(m_map, m_finance),
          m_market(m_map, m_trains, m_finance)
    {
        InitializeSampleData();
    }

    void GameSimulation::InitializeSampleData() {
        m_map.InitializeSampleData();
        m_trains.InitializeSampleData();

        for (auto& station : m_map.GetStations()) {
            m_market.RegenerateStationMarket(station);
        }

        m_selected_station_id = "STN_CENTRAL";
    }

    void GameSimulation::Update(float deltaTime) {
        // Station Market Regeneration Tick
        for (auto& station : m_map.GetStations()) {
            station.regen_timer += deltaTime;
            if (station.regen_timer >= station.regen_interval) {
                station.regen_timer = 0.0f;
                m_market.RegenerateStationMarket(station);
            }
        }

        // Sub-managers mutate state internally via their stored references
        m_trains.Update(deltaTime);
    }

} // namespace app