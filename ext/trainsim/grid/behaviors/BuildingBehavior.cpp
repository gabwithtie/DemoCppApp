#include "BuildingBehavior.h"

#include "trainsim/grid/BuildingTypes.h"

#include <magic_enum.hpp>

#include "trainsim/grid/GridManager.h"

namespace Trainsim {
    int BuildingBehavior::ReceiveMessage(GridCell* cell, const CellMessage& message) {
        if (!cell) return 0;

        // Base implementation for universal building commands
        if (message.type == "destroy") {
			auto oldType = cell->type; // Cache the old type for GridManager update

            cell->type = std::string(magic_enum::enum_name(BuildingType::EMPTY));
            cell->data.clear(); // Wipe out any lingering parameters so the next building starts fresh

            GridManager::Get()->UpdateCell(cell, oldType);

            return 1;
        }

        return 0;
    }

    std::vector<CellMessage> BuildingBehavior::GetMessageList() const {
        return { {
            .label = "Destroy Structure",
            .type = "destroy",
            .data = {}
        } };
    }
}