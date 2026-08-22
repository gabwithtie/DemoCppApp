#include "TrainManager.hpp"
#include "map/MapManager.hpp"
#include "finance/FinanceManager.hpp"
#include <cmath>

namespace app {

    TrainManager::TrainManager(MapManager& map, FinanceManager& finance)
        : m_map(map), m_finance(finance) {}

    void TrainManager::InitializeSampleData() {
        Train train1;
        train1.id = "EXPRESS-01";
        train1.speed = 110.0f;
        train1.current_station_id = "STN_WEST";
        train1.carriages = {
            { "CAR-01", { {"passenger:STN_EAST", 12} } }
        };

        m_trains = { train1 };
    }

    const Train* TrainManager::FindTrain(const std::string& id) const {
        for (const auto& train : m_trains) {
            if (train.id == id) return &train;
        }
        return nullptr;
    }

    Train* TrainManager::FindTrainMutable(const std::string& id) {
        for (auto& train : m_trains) {
            if (train.id == id) return &train;
        }
        return nullptr;
    }

    bool TrainManager::SetTrainDestination(const std::string& train_id, const std::string& target_station_id) {
        for (auto& train : m_trains) {
            if (train.id == train_id) {
                std::string start_from = train.active_path.empty() ? train.current_station_id : train.active_path[train.current_segment];
                auto path = m_map.FindPath(start_from, target_station_id);

                if (!path.empty()) {
                    train.target_station_id = target_station_id;
                    train.active_path = path;
                    train.current_segment = 0;
                    train.t = 0.0f;
                    return true;
                }
                break;
            }
        }
        return false;
    }

    gbe::Vector2 TrainManager::GetTrainWorldPosition(const Train& train) const {
        if (train.active_path.size() < 2 || train.current_segment >= static_cast<int>(train.active_path.size()) - 1) {
            const Station* st = m_map.FindStation(train.current_station_id);
            return st ? st->position : gbe::Vector2::zero;
        }

        const Station* src = m_map.FindStation(train.active_path[train.current_segment]);
        const Station* dst = m_map.FindStation(train.active_path[train.current_segment + 1]);

        if (!src || !dst) return gbe::Vector2::zero;

        return gbe::Vector2(
            src->position.x + (dst->position.x - src->position.x) * train.t,
            src->position.y + (dst->position.y - src->position.y) * train.t
        );
    }

    void TrainManager::Update(float deltaTime) {
        for (auto& train : m_trains) {
            if (train.active_path.size() < 2) continue;

            if (train.current_segment >= static_cast<int>(train.active_path.size()) - 1) {
                train.current_station_id = train.active_path.back();
                train.target_station_id.clear();
                train.active_path.clear();
                train.current_segment = 0;
                train.t = 0.0f;
                continue;
            }

            const Station* src = m_map.FindStation(train.active_path[train.current_segment]);
            const Station* dst = m_map.FindStation(train.active_path[train.current_segment + 1]);
            if (!src || !dst) continue;

            float dx = dst->position.x - src->position.x;
            float dy = dst->position.y - src->position.y;
            float seg_dist = std::sqrt(dx * dx + dy * dy);

            if (seg_dist <= 0.001f) {
                train.current_segment++;
                train.t = 0.0f;
                continue;
            }

            float distance_moved = train.speed * deltaTime;
            m_finance.DeductElectricityCost(distance_moved);

            train.t += distance_moved / seg_dist;

            while (train.t >= 1.0f) {
                train.t -= 1.0f;
                train.current_segment++;

                if (train.current_segment >= static_cast<int>(train.active_path.size()) - 1) {
                    train.current_station_id = train.active_path.back();
                    train.target_station_id.clear();
                    train.active_path.clear();
                    train.current_segment = 0;
                    train.t = 0.0f;
                    break;
                }

                src = m_map.FindStation(train.active_path[train.current_segment]);
                dst = m_map.FindStation(train.active_path[train.current_segment + 1]);
                if (!src || !dst) break;

                dx = dst->position.x - src->position.x;
                dy = dst->position.y - src->position.y;
                seg_dist = std::sqrt(dx * dx + dy * dy);
                if (seg_dist <= 0.001f) break;
            }
        }
    }

} // namespace app