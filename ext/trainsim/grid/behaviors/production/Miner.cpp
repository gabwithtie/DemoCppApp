#include "Miner.h"
#include "trainsim/grid/GridManager.h"
#include "trainsim/resource/ItemTypes.h"

#include <magic_enum.hpp>

namespace Trainsim {

    int Miner::CalculateMiningSpeed(int x, int y, int seed, int itemType) const {
        // Simple deterministic hash mixing coordinates, global seed, and material ID
        unsigned int hash = (seed * 73856093) ^ (x * 19349663) ^ (y * 83492791) ^ (itemType * 39916801);

        // Map the resulting hash to a normalized float factor (0.0 to 1.0)
        float normalized = static_cast<float>(hash % 1000) / 1000.0f;

        // Example: speeds range randomly from 0.5 to 3.0 items per update
        return 1 + (normalized * 3.0f);
    }

    void Miner::SetTargetItem(GridCell* cell, int itemType) {
        if (!cell) return;

        // Wipe current output and item inventories to prevent free material transformations 
        for (auto it = cell->data.begin(); it != cell->data.end(); ) {
            if (it->first.rfind("out::", 0) == 0 || it->first.rfind("i::", 0) == 0) {
                it = cell->data.erase(it);
            }
            else {
                ++it;
            }
        }

        cell->data["target_item"] = static_cast<float>(itemType);

        // Fetch positional/world context to generate the deterministic speed rate
        GridManager* manager = GridManager::Get();
        if (manager) {
            int cx = 0, cy = 0;
            if (manager->FindCellCoordinates(cell, cx, cy)) {
                int seed = manager->GetGridData().seed;
                cell->data["mining_speed"] = CalculateMiningSpeed(cx, cy, seed, itemType);
            }
        }
    }

    void Miner::Update(GridCell* cell) {
        if (!cell) return;

        // Ensure defaults are populated upon initial placement
        if (cell->data.find("target_item") == cell->data.end()) {
            SetTargetItem(cell, static_cast<int>(ItemType::IRON));
        }
        if (cell->data.find("net_power") == cell->data.end()) {
			cell->data["net_power"] = -1.0f;
        }

        // Run the base production tick sequence (which consumes inputs and generates outputs)
        ProductionBehavior::Update(cell);
    }

    std::map<int, int> Miner::GetRecipeInputs(const GridCell* cell) const {
        // Miners don't require external inputs to function; return empty
        return {};
    }

    std::pair<int, int> Miner::GetRecipeOutput(const GridCell* cell) const {
        if (!cell || cell->data.find("target_item") == cell->data.end()) return {};

        int target = static_cast<int>(cell->data.at("target_item"));
        int speed = cell->data.count("mining_speed") ? cell->data.at("mining_speed") : 0.0f;

        // Return the configured target yield amount per update frame
        return { target, speed };
    }

    int Miner::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Configuration action sent from the ImGui contextual editor panel
        if (message.type == "set_mining_target") {
            auto it = message.data.find("item_type");
            if (it != message.data.end()) {
                int itemType = static_cast<int>(it->second);
                SetTargetItem(cell, itemType);
                return 1;
            }
            return 0;
        }

        // Allow ProductionBehavior to process extraction checks ("is_production", "withdraw", etc.)
        return ProductionBehavior::ReceiveMessage(cell, message);
    }

    std::vector<CellMessage> Miner::GetMessageList() const {
        // Bring in default layout actions ("destroy", etc.)
        std::vector<CellMessage> messages = ProductionBehavior::GetMessageList();

        // Loop over the minable ore items strictly bounded within the enum layout boundaries
        int start = static_cast<int>(ItemType::START_MINED) + 1;
        int end = static_cast<int>(ItemType::END_MINED);

        for (int i = start; i < end; ++i) {
            std::string itemName = std::string(magic_enum::enum_name(static_cast<ItemType>(i)));

            messages.push_back({
                .label = "Set Target: " + itemName,
                .type = "set_mining_target",
                .data = { {"item_type", static_cast<float>(i)} }
                });
        }

        return messages;
    }
}