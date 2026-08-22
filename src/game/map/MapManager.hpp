#pragma once
#include "TrainGameModels.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace app {

    class MapManager {
    public:
        void InitializeSampleData();
        void RebuildGraph();

        const Station* FindStation(const std::string& id) const;
        Station* FindStationMutable(const std::string& id);
        std::vector<std::string> FindPath(const std::string& start_id, const std::string& end_id) const;

        std::vector<Station>& GetStations() { return m_stations; }
        const std::vector<Station>& GetStations() const { return m_stations; }
        const std::vector<TrackLine>& GetLines() const { return m_lines; }

    private:
        std::vector<Station> m_stations;
        std::vector<TrackLine> m_lines;
        std::unordered_map<std::string, size_t> m_station_map;
        std::unordered_map<std::string, std::vector<std::string>> m_graph;
    };

} // namespace app