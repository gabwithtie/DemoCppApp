// Territory.cpp
#include "Territory.h"
#include "trainsim/grid/BuildingTypes.h"
#include "trainsim/grid/GridManager.h"

#include <magic_enum.hpp>

namespace Trainsim {
    void Territory::Update(GridCell* cell) {
        // Idle or ambient state updates go here
    }

    int Territory::ReceiveMessage(GridCell* cell, const CellMessage& message) {
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

    std::vector<CellMessage> Territory::GetMessageList() const {
        std::vector<CellMessage> totalcreateoptions;

        for (int key = static_cast<int>(BuildingType::EMPTY) + 1; key < static_cast<int>(BuildingType::MAX); key++)
        {
            BuildingType c1 = static_cast<BuildingType>(key);
            std::string label(magic_enum::enum_name(c1));

            totalcreateoptions.push_back({
                    .label = "create " + label,
                    .type = "CreateBuilding",
                    .data = { {"buildingtype", key} }
                });
        }

        return totalcreateoptions;
    }
}