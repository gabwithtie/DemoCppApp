#include "MarketManager.hpp"
#include "map/MapManager.hpp"
#include "train/TrainManager.hpp"
#include "finance/FinanceManager.hpp"
#include <cstdlib>
#include <algorithm>

namespace app {

    MarketManager::MarketManager(MapManager& map, TrainManager& trains, FinanceManager& finance)
        : m_map(map), m_trains(trains), m_finance(finance) 
    {
        m_cargo_definitions = {
            { "grain", "Grain", 15.0 },
            { "coal", "Coal", 32.0 },
            { "steel", "Steel Ingot", 85.0 },
            { "electronics", "Electronics", 160.0 }
        };
    }

    void MarketManager::RegenerateStationMarket(Station& station) {
        // 1. Generate Passengers targeting a different station
        std::vector<std::string> valid_destinations;
        for (const auto& st : m_map.GetStations()) {
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
        float variance = 0.8f + static_cast<float>(rand() % 40) / 100.0f;

        bool found_cargo = false;
        for (auto& item : station.market) {
            if (item.item_id == cargo.id) {
                item.quantity += 2 + (rand() % 6);
                item.buy_price = cargo.base_price * variance;
                item.sell_price = item.buy_price * 0.85;
                found_cargo = true;
                break;
            }
        }

        if (!found_cargo) {
            double buy = cargo.base_price * variance;
            station.market.push_back({ cargo.id, 5 + (rand() % 15), buy, buy * 0.85 });
        }
    }

    double MarketManager::GetItemSellPriceAtStation(const Station& station, const std::string& item_id) const {
        if (item_id.rfind("passenger:", 0) == 0) {
            std::string dest_id = item_id.substr(10);
            if (dest_id == station.id) {
                return 50.0;
            }
            return 10.0;
        }

        for (const auto& item : station.market) {
            if (item.item_id == item_id) {
                return item.sell_price;
            }
        }

        for (const auto& def : m_cargo_definitions) {
            if (def.id == item_id) {
                return def.base_price * 0.85;
            }
        }

        return 10.0;
    }

    bool MarketManager::BuyCargoFromStation(const std::string& station_id, const std::string& train_id, size_t carriage_idx, const std::string& item_id, int quantity) {
        if (quantity <= 0) return false;

        Station* station = m_map.FindStationMutable(station_id);
        Train* train = m_trains.FindTrainMutable(train_id);

        if (!station || !train) return false;

        if (train->current_station_id != station_id || !train->target_station_id.empty()) {
            return false;
        }

        if (carriage_idx >= train->carriages.size()) return false;

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

        m_finance.AddExpense(total_cost, "Bought " + std::to_string(quantity) + "x " + item_id + " at " + station->name);
        market_item->quantity -= quantity;

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

    bool MarketManager::SellCargoToStation(const std::string& train_id, size_t carriage_idx, const std::string& station_id, const std::string& item_id, int quantity) {
        if (quantity <= 0) return false;

        Station* station = m_map.FindStationMutable(station_id);
        Train* train = m_trains.FindTrainMutable(train_id);

        if (!station || !train) return false;

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

        m_finance.AddIncome(total_revenue, "Sold " + std::to_string(quantity) + "x " + item_id + " to " + station->name, 0.0f);

        inv_it->quantity -= quantity;
        if (inv_it->quantity <= 0) {
            inventory.erase(inv_it);
        }

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