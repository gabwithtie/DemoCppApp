#include "PowerMain.h"
#include "trainsim/grid/BuildingTypes.h"
#include "trainsim/grid/GridManager.h"
#include <magic_enum.hpp>
#include <set>

namespace Trainsim {

    float PowerMain::GetMaxRadius(const GridCell* cell) const {
        return 10.0f; // Maximum distance to search for a terminal
    }

    std::string PowerMain::GetTargetBuildingType() const {
        return std::string(magic_enum::enum_name(BuildingType::POWER_TERMINAL));
    }

    void PowerMain::NotifyTerminal(int tx, int ty, const std::string& messageType) {
        GridManager* manager = GridManager::Get();
        if (manager) {
            CellMessage msg;
            msg.type = messageType;
            manager->SendMessageToCell(tx, ty, msg);
        }
    }

    void PowerMain::Update(GridCell* cell) {
        if (!cell) return;

        // 1. Read PREVIOUS connections from our cache
        std::set<std::pair<int, int>> previousConnections;
        int index = 0;
        while (true) {
            std::string keyX = "target::" + std::to_string(index) + "::x";
            std::string keyY = "target::" + std::to_string(index) + "::y";
            if (cell->data.find(keyX) == cell->data.end()) break;

            previousConnections.insert({ static_cast<int>(cell->data[keyX]), static_cast<int>(cell->data[keyY]) });
            index++;
        }

        // Let the base line behavior do its standard targeting math
        LineBehavior::Update(cell);

        // 2. Read CURRENT connections found by LineBehavior
        std::set<std::pair<int, int>> currentConnections;
        index = 0;
        while (true) {
            std::string keyX = "c::" + std::to_string(index) + "::x";
            std::string keyY = "c::" + std::to_string(index) + "::y";
            if (cell->data.find(keyX) == cell->data.end()) break;

            currentConnections.insert({ static_cast<int>(cell->data[keyX]), static_cast<int>(cell->data[keyY]) });
            index++;
        }

        // 3. Find NEW connections (In Current, but not in Previous)
        for (const auto& conn : currentConnections) {
            if (previousConnections.find(conn) == previousConnections.end()) {
                NotifyTerminal(conn.first, conn.second, "connect_main");
            }
        }

        // 4. Find LOST connections (In Previous, but not in Current)
        for (const auto& conn : previousConnections) {
            if (currentConnections.find(conn) == currentConnections.end()) {
                NotifyTerminal(conn.first, conn.second, "disconnect_main");
            }
        }

        // 5. Rebuild the Cache
        // Wipe old cache keys
        index = 0;
        while (true) {
            std::string keyX = "target::" + std::to_string(index) + "::x";
            std::string keyY = "target::" + std::to_string(index) + "::y";
            if (cell->data.find(keyX) == cell->data.end()) break;
            cell->data.erase(keyX);
            cell->data.erase(keyY);
            index++;
        }

        // Write new cache keys
        index = 0;
        for (const auto& conn : currentConnections) {
            std::string keyX = "target::" + std::to_string(index) + "::x";
            std::string keyY = "target::" + std::to_string(index) + "::y";
            cell->data[keyX] = static_cast<float>(conn.first);
            cell->data[keyY] = static_cast<float>(conn.second);
            index++;
        }
    }

    int PowerMain::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // If the player deletes the PowerMain line, disconnect from all cached terminals
        if (message.type == "destroy") {
            int index = 0;
            while (true) {
                std::string keyX = "target::" + std::to_string(index) + "::x";
                std::string keyY = "target::" + std::to_string(index) + "::y";
                if (cell->data.find(keyX) == cell->data.end()) break;

                int tx = static_cast<int>(cell->data[keyX]);
                int ty = static_cast<int>(cell->data[keyY]);
                NotifyTerminal(tx, ty, "disconnect_main");

                index++;
            }
        }

        return LineBehavior::ReceiveMessage(cell, message);
    }
}