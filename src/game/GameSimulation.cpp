#include "GameSimulation.hpp"
#include <cmath>
#include <queue>
#include <algorithm>
#include <cstdlib>

namespace app {

    GameSimulation::GameSimulation() {
        // Fallback cargo type definitions matching cargo_types.json
        m_cargo_definitions = {
            { "grain", "Grain", 15.0 },
            { "coal", "Coal", 32.0 },
            { "steel", "Steel Ingot", 85.0 },
            { "electronics", "Electronics", 160.0 }
        };

        InitializeSampleData();
    }

    void GameSimulation::InitializeSampleData() {
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

        // Seed initial market inventory for all stations
        for (auto& station : m_stations) {
            RegenerateStationMarket(station);
        }

        m_selected_station_id = "STN_CENTRAL";

        Train train1;
        train1.id = "EXPRESS-01";
        train1.speed = 110.0f;
        train1.current_station_id = "STN_WEST";
        train1.carriages = {
            { "CAR-01", { {"passenger:STN_EAST", 12} } }
        };

        m_trains = { train1 };
    }

    void GameSimulation::RebuildGraph() {
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

    const Station* GameSimulation::FindStation(const std::string& id) const {
        auto it = m_station_map.find(id);
        return (it != m_station_map.end()) ? &m_stations[it->second] : nullptr;
    }

    Station* GameSimulation::FindStationMutable(const std::string& id) {
        auto it = m_station_map.find(id);
        return (it != m_station_map.end()) ? &m_stations[it->second] : nullptr;
    }

    const Train* GameSimulation::FindTrain(const std::string& id) const {
        for (const auto& train : m_trains) {
            if (train.id == id) return &train;
        }
        return nullptr;
    }

    Train* GameSimulation::FindTrainMutable(const std::string& id) {
        for (auto& train : m_trains) {
            if (train.id == id) return &train;
        }
        return nullptr;
    }

    void GameSimulation::RegenerateStationMarket(Station& station) {
        // 1. Generate Passengers targeting a different station
        std::vector<std::string> valid_destinations;
        for (const auto& st : m_stations) {
            if (st.id != station.id) valid_destinations.push_back(st.id);
        }

        if (!valid_destinations.empty()) {
            std::string dest_id = valid_destinations[rand() % valid_destinations.size()];
            std::string passenger_key = "passenger:" + dest_id;

            bool found = false;
            for (auto& item : station.market) {
                if (item.item_id == passenger_key) {
                    item.quantity += 3 + (rand() % 8);
                    found = true;
                    break;
                }
            }
            if (!found) {
                station.market.push_back({ passenger_key, 5 + (rand() % 10), 0.0, 0.0 });
            }
        }

        // 2. Generate/Fluctuate Cargo Items
        const auto& cargo = m_cargo_definitions[rand() % m_cargo_definitions.size()];
        float variance = 0.8f + static_cast<float>(rand() % 40) / 100.0f; // 0.8x - 1.2x price multiplier

        bool found_cargo = false;
        for (auto& item : station.market) {
            if (item.item_id == cargo.id) {
                item.quantity += 2 + (rand() % 6);
                item.buy_price = cargo.base_price * variance;
                item.sell_price = item.buy_price * 0.85; // 15% margin
                found_cargo = true;
                break;
            }
        }

        if (!found_cargo) {
            double buy = cargo.base_price * variance;
            station.market.push_back({ cargo.id, 5 + (rand() % 15), buy, buy * 0.85 });
        }
    }

    std::vector<std::string> GameSimulation::FindPath(const std::string& start_id, const std::string& end_id) const {
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

    bool GameSimulation::SetTrainDestination(const std::string& train_id, const std::string& target_station_id) {
        for (auto& train : m_trains) {
            if (train.id == train_id) {
                std::string start_from = train.active_path.empty() ? train.current_station_id : train.active_path[train.current_segment];
                auto path = FindPath(start_from, target_station_id);

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

    gbe::Vector2 GameSimulation::GetTrainWorldPosition(const Train& train) const {
        if (train.active_path.size() < 2 || train.current_segment >= static_cast<int>(train.active_path.size()) - 1) {
            const Station* st = FindStation(train.current_station_id);
            return st ? st->position : gbe::Vector2::zero;
        }

        const Station* src = FindStation(train.active_path[train.current_segment]);
        const Station* dst = FindStation(train.active_path[train.current_segment + 1]);

        if (!src || !dst) return gbe::Vector2::zero;

        return gbe::Vector2(
            src->position.x + (dst->position.x - src->position.x) * train.t,
            src->position.y + (dst->position.y - src->position.y) * train.t
        );
    }

    void GameSimulation::Update(float deltaTime) {
        // Station Market Regeneration Tick
        for (auto& station : m_stations) {
            station.regen_timer += deltaTime;
            if (station.regen_timer >= station.regen_interval) {
                station.regen_timer = 0.0f;
                RegenerateStationMarket(station);
            }
        }

        // Train Positions & Movement
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

            const Station* src = FindStation(train.active_path[train.current_segment]);
            const Station* dst = FindStation(train.active_path[train.current_segment + 1]);
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

                src = FindStation(train.active_path[train.current_segment]);
                dst = FindStation(train.active_path[train.current_segment + 1]);
                if (!src || !dst) break;

                dx = dst->position.x - src->position.x;
                dy = dst->position.y - src->position.y;
                seg_dist = std::sqrt(dx * dx + dy * dy);
                if (seg_dist <= 0.001f) break;
            }
        }
    }

    bool GameSimulation::BuyCargoFromStation(const std::string& station_id, const std::string& train_id, size_t carriage_idx, const std::string& item_id, int quantity) {
        if (quantity <= 0) return false;

        Station* station = FindStationMutable(station_id);
        Train* train = FindTrainMutable(train_id);

        if (!station || !train) return false;

        // Validation: Train must be stopped at the target station
        if (train->current_station_id != station_id || !train->target_station_id.empty()) {
            return false;
        }

        if (carriage_idx >= train->carriages.size()) return false;

        // Locate market item
        MarketItem* market_item = nullptr;
        for (auto& item : station->market) {
            if (item.item_id == item_id) {
                market_item = &item;
                break;
            }
        }

        if (!market_item || market_item->quantity < quantity) return false;

        double total_cost = market_item->buy_price * static_cast<double>(quantity);
        if (m_finance.GetBalance() < total_cost) return false;

        // Deduct Funds & Register Expense
        m_finance.AddExpense(total_cost, "Bought " + std::to_string(quantity) + "x " + item_id + " at " + station->name);

        // Deduct Station Stock
        market_item->quantity -= quantity;

        // Transfer to Carriage Inventory
        auto& inventory = train->carriages[carriage_idx].inventory;
        bool found_inv = false;
        for (auto& inv_item : inventory) {
            if (inv_item.item_id == item_id) {
                inv_item.quantity += quantity;
                found_inv = true;
                break;
            }
        }

        if (!found_inv) {
            inventory.push_back({ item_id, quantity });
        }

        return true;
    }

    double GameSimulation::GetItemSellPriceAtStation(const Station& station, const std::string& item_id) const {
        // Handling passenger payouts based on destination
        if (item_id.rfind("passenger:", 0) == 0) {
            std::string dest_id = item_id.substr(10);
            if (dest_id == station.id) {
                return 50.0; // Full fare payout at destination
            }
            return 10.0; // Reduced transfer fare
        }

        // Commodity lookup in station market
        for (const auto& item : station.market) {
            if (item.item_id == item_id) {
                return item.sell_price;
            }
        }

        // Default base price fallback if item not actively traded
        for (const auto& def : m_cargo_definitions) {
            if (def.id == item_id) {
                return def.base_price * 0.85;
            }
        }

        return 10.0;
    }

    bool GameSimulation::SellCargoToStation(const std::string& train_id, size_t carriage_idx, const std::string& station_id, const std::string& item_id, int quantity) {
        if (quantity <= 0) return false;

        Station* station = FindStationMutable(station_id);
        Train* train = FindTrainMutable(train_id);

        if (!station || !train) return false;

        // Train must be stopped at target station
        if (train->current_station_id != station_id || !train->target_station_id.empty()) {
            return false;
        }

        if (carriage_idx >= train->carriages.size()) return false;

        auto& inventory = train->carriages[carriage_idx].inventory;
        auto inv_it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryItem& item) {
            return item.item_id == item_id;
            });

        if (inv_it == inventory.end() || inv_it->quantity < quantity) return false;

        double unit_price = GetItemSellPriceAtStation(*station, item_id);
        double total_revenue = unit_price * static_cast<double>(quantity);

        // Credit Company Funds
        m_finance.AddIncome(total_revenue, "Sold " + std::to_string(quantity) + "x " + item_id + " to " + station->name, 0.0f);

        // Deduct from Train Carriage
        inv_it->quantity -= quantity;
        if (inv_it->quantity <= 0) {
            inventory.erase(inv_it);
        }

        // Replenish station cargo market if not passenger
        if (item_id.rfind("passenger:", 0) != 0) {
            bool found = false;
            for (auto& market_item : station->market) {
                if (market_item.item_id == item_id) {
                    market_item.quantity += quantity;
                    found = true;
                    break;
                }
            }
            if (!found) {
                station->market.push_back({ item_id, quantity, unit_price * 1.15, unit_price });
            }
        }

        return true;
    }

} // namespace app