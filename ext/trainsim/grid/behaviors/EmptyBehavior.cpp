// EmptyBehavior.cpp
#include "EmptyBehavior.h"
#include "trainsim/grid/BuildingTypes.h"
#include "trainsim/grid/GridManager.h"

#include <magic_enum.hpp>

namespace Trainsim {
    void EmptyBehavior::Update(GridCell* cell) {
        // Idle or ambient state updates go here
    }

    int EmptyBehavior::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (message.type == "CreateBuilding") {
            auto it = message.data.find("buildingtype");
            if (it != message.data.end()) {
                int typeId = static_cast<int>(it->second);

                if (typeId < static_cast<int>(BuildingType::MAX)) {
					auto typeEnum = static_cast<BuildingType>(typeId);

                    cell->type = std::string(magic_enum::enum_name(typeEnum));
                    cell->data.clear(); // Wipe out default null state variables

                    GridManager::Get()->UpdateCell(cell, std::string(magic_enum::enum_name(BuildingType::EMPTY)));
                }

                return 1;
            }
        }

        return 0;
    }

    std::vector<CellMessage> EmptyBehavior::GetMessageList() const {
        std::vector<CellMessage> totalcreateoptions;

		auto label = std::string(magic_enum::enum_name(BuildingType::HEADQUARTERS));
		auto key = static_cast<int>(BuildingType::HEADQUARTERS);

        //Just the base
        totalcreateoptions.push_back({
                    .label = "create " + label,
                    .type = "CreateBuilding",
                    .data = { {"buildingtype", key} }
            });

        return totalcreateoptions;
    }
}