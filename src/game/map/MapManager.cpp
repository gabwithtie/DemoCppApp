#include "MapManager.hpp"
#include <queue>
#include <algorithm>

namespace app {

    void MapManager::InitializeSampleData() {
        m_stations = {
            { "STN_WEST",    "West Junction",   gbe::Vector2(-200.0f, 0.0f) },
            { "STN_CENTRAL", "Central Station", gbe::Vector2(-50.0f, 0.0f) },
            { "STN_NORTH",   "North Terminal",  gbe::Vector2(50.0f, -100.0f) },
            { "STN_EAST",    "East Depot",      gbe::Vector2(180.0f, 0.0f) }
        };

        m_lines = {
            { "LINE_BLUE", "Main Line", 0xFFE68032, { "STN_WEST", "STN_CENTRAL", "STN_EAST" } },
            { "LINE_RED",  "North Spur", 0xFF3232E6, { "STN_CENTRAL", "STN_NORTH", "STN_EAST" } }
        };

        RebuildGraph();
    }

    void MapManager::RebuildGraph() {
        m_station_map.clear();
        m_graph.clear();

        for (size_t i = 0; i < m_stations.size(); ++i) {
            m_station_map[m_stations[i].id] = i;
        }

        for (const auto& line : m_lines) {
            for (size_t i = 0; i + 1 < line.station_ids.size(); ++i) {
                const std::string& u = line.station_ids[i];
                const std::string& v = line.station_ids[i + 1];
                m_graph[u].push_back(v);
                m_graph[v].push_back(u);
            }
        }
    }

    const Station* MapManager::FindStation(const std::string& id) const {
        auto it = m_station_map.find(id);
        return (it != m_station_map.end()) ? &m_stations[it->second] : nullptr;
    }

    Station* MapManager::FindStationMutable(const std::string& id) {
        auto it = m_station_map.find(id);
        return (it != m_station_map.end()) ? &m_stations[it->second] : nullptr;
    }

    std::vector<std::string> MapManager::FindPath(const std::string& start_id, const std::string& end_id) const {
        if (start_id == end_id) return { start_id };

        std::queue<std::string> q;
        std::unordered_map<std::string, std::string> parent;

        q.push(start_id);
        parent[start_id] = "";

        bool found = false;
        while (!q.empty()) {
            std::string current = q.front();
            q.pop();

            if (current == end_id) {
                found = true;
                break;
            }

            auto it = m_graph.find(current);
            if (it != m_graph.end()) {
                for (const auto& neighbor : it->second) {
                    if (parent.find(neighbor) == parent.end()) {
                        parent[neighbor] = current;
                        q.push(neighbor);
                    }
                }
            }
        }

        if (!found) return {};

        std::vector<std::string> path;
        for (std::string curr = end_id; !curr.empty(); curr = parent[curr]) {
            path.push_back(curr);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

} // namespace app