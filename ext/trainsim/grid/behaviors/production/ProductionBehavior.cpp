#include "ProductionBehavior.h"
#include <string>
#include <algorithm>

namespace Trainsim {
    float ProductionBehavior::GetMaxStorage(GridCell* cell) {
        if (!cell->data.count("max_storage"))
            cell->data["max_storage"] = 100.0f; // Sane default cap

        return cell->data.at("max_storage");
    }

    void ProductionBehavior::Update(GridCell* cell) {
        if (!cell) return;

        if (cell->data.find("health") == cell->data.end()) {
            cell->data["health"] = 0.0f;
        }

        if (cell->data["health"] < 1.0f) {
            return;
        }

        if (cell->data["powered"] == 0.0f && cell->data["net_power"] < 0.0f)
            return;

        auto inputs = GetRecipeInputs(cell);
        auto output = GetRecipeOutput(cell);
        float max_storage = GetMaxStorage(cell);

        bool canProduce = true;

        // 1. Check if all required inputs are present using the standardized i:: format
        for (auto const& [type, amount] : inputs) {
            std::string key = "i::" + std::to_string(type);
            if (cell->data.find(key) == cell->data.end() || cell->data[key] < amount) {
                canProduce = false;
                break;
            }
        }

        // 2. Check if the unified storage buffer has enough room for the output
        if (canProduce && output.second > 0.0f) {
            int totalUsed = GetTotalStorageUsed(cell);
            if (totalUsed + output.second > max_storage) {
                canProduce = false;
            }
        }

        // 3. Consume inputs and generate the output using StorageBehavior's safe methods
        if (canProduce) {
            for (auto const& [type, amount] : inputs) {
                Withdraw(cell, type, static_cast<int>(amount));
            }
            if (output.second > 0.0f) {
                Deposit(cell, output.first, static_cast<int>(output.second));
            }
        }
    }

    int ProductionBehavior::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // --- MANIFOLD ROUTING IDENTIFIERS ---
        if (message.type == "is_production_input") {
            auto typeIt = message.data.find("item_type");
            if (typeIt != message.data.end()) {
                int itemType = static_cast<int>(typeIt->second);
                auto inputs = GetRecipeInputs(cell);
                if (inputs.find(itemType) != inputs.end()) {
                    return 1; // Compatible production input
                }
            }
            return -1; // Production building, but does not accept this item type
        }

        if (message.type == "get_production_output") {
            auto output = GetRecipeOutput(cell);
            if (output.second > 0.0f) {
                return output.first + 1; // Offset by +1 to distinguish from 0 (unhandled)
            }
            return -1; // Production building, but currently has no output
        }

        // --- FILTERED STORAGE INTERACTIONS ---

        // Handle raw material intakes
        if (message.type == "deposit") {
            auto typeIt = message.data.find("item_type");
            if (typeIt == message.data.end()) typeIt = message.data.find("deposit_type");

            if (typeIt != message.data.end()) {
                int itemType = static_cast<int>(typeIt->second);
                auto inputs = GetRecipeInputs(cell);

                // FILTER: Only allow deposits for items that are explicitly part of our recipe!
                // If it is, pass the message up to StorageBehavior to safely execute the physical transaction.
                if (inputs.find(itemType) != inputs.end()) {
                    return StorageBehavior::ReceiveMessage(cell, message);
                }
            }
            return 0; // Reject deposit
        }

        // Handle output extractions
        if (message.type == "withdraw") {
            auto output = GetRecipeOutput(cell);
            auto typeIt = message.data.find("item_type");
            if (typeIt == message.data.end()) typeIt = message.data.find("withdraw_type");

            if (typeIt != message.data.end()) {
                int itemType = static_cast<int>(typeIt->second);

                // FILTER: Only allow withdrawals of our finished product. Never let routers pull our input ingredients!
                if (itemType == output.first) {
                    return StorageBehavior::ReceiveMessage(cell, message);
                }
                return 0; // Reject withdraw
            }
            else {
                // If the withdraw request was generic (no type specified), forcefully restrict it to our output type 
                // so StorageBehavior doesn't accidentally pull an input ingredient instead.
                CellMessage restrictedMessage = message;
                restrictedMessage.data["item_type"] = static_cast<float>(output.first);
                return StorageBehavior::ReceiveMessage(cell, restrictedMessage);
            }
        }

        // Fall back to universal behavior handling (processes destroy, etc.)
        return StorageBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> ProductionBehavior::GetMessageList() const {
        return StorageBehavior::GetMessageList();
    }
}