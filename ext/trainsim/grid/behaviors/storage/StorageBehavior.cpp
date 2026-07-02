// StorageBehavior.cpp
#include "StorageBehavior.h"
#include <string>
#include <algorithm>

namespace Trainsim {
    int StorageBehavior::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Handle programmatic deposits
        if (message.type == "deposit") {
            auto typeIt = message.data.find("item_type");
            if (typeIt == message.data.end()) typeIt = message.data.find("deposit_type"); // resilient fallback

            auto amountIt = message.data.find("amount");
            if (amountIt == message.data.end()) amountIt = message.data.find("deposit_amount"); // resilient fallback

            if (typeIt != message.data.end() && amountIt != message.data.end()) {
                int itemType = static_cast<int>(typeIt->second);
                int amount = static_cast<int>(amountIt->second);

                return Deposit(cell, itemType, amount); // Return the exact quantity successfully allocated
            }
            return 0;
        }

        // Handle programmatic withdrawals
        if (message.type == "withdraw") {
            auto typeIt = message.data.find("item_type");
            if (typeIt == message.data.end()) typeIt = message.data.find("withdraw_type"); // resilient fallback

            auto amountIt = message.data.find("amount");
            if (amountIt == message.data.end()) amountIt = message.data.find("withdraw_amount"); // resilient fallback

            if (amountIt != message.data.end()) {
                int amount = static_cast<int>(amountIt->second);

                // Scenario A: Caller requests a highly specific item type channel
                if (typeIt != message.data.end()) {
                    int itemType = static_cast<int>(typeIt->second);
                    return Withdraw(cell, itemType, amount);
                }
                // Scenario B: Pull the first available item found (generic bulk routing)
                else {
                    for (auto const& [key, val] : cell->data) {
                        if (key.rfind("i::", 0) == 0 && val > 0.0f) {
                            int itemType = std::stoi(key.substr(3));
                            return Withdraw(cell, itemType, amount);
                        }
                    }
                }
            }
            return 0;
        }

        // Fallback to BuildingBehavior for structural defaults like "destroy"
        return BuildingBehavior::ReceiveMessage(cell, message);
    }

    int StorageBehavior::GetTotalStorageUsed(const GridCell* cell) const {
        int total = 0;
        for (auto const& [key, val] : cell->data) {
            if (key.rfind("i::", 0) == 0) { // Fast prefix check string validation match
                total += static_cast<int>(val);
            }
        }
        return total;
    }

    int StorageBehavior::Deposit(GridCell* cell, int itemtype, int amount) {
        if (amount <= 0) return 0;

        int currentTotal = GetTotalStorageUsed(cell);
        float max_storage = cell->data.count("max_storage") ? cell->data["max_storage"] : 0.0f;

        if (currentTotal < static_cast<int>(max_storage)) {
            auto final_amount = (int)std::fmin(max_storage - currentTotal, amount);

            std::string key = "i::" + std::to_string(itemtype);
            cell->data[key] += static_cast<float>(final_amount);
            return final_amount;
        }
        return 0;
    }

    int StorageBehavior::Withdraw(GridCell* cell, int itemtype, int amount) {
        if (amount <= 0) return 0;

        std::string key = "i::" + std::to_string(itemtype);
        auto it = cell->data.find(key);
        if (it != cell->data.end()) {
            int available = static_cast<int>(it->second);
            int taken = std::min(amount, available);

            cell->data[key] -= static_cast<float>(taken);

            // Cleanup check constraint rule validation step
            if (cell->data[key] <= 0.0f) {
                cell->data.erase(it);
            }
            return taken;
        }
        return 0;
    }
}