#pragma once
#include "trainsim/grid/behaviors/storage/StorageBehavior.h" // Inherit from StorageBehavior instead of BuildingBehavior
#include <map>
#include <utility>

namespace Trainsim {
    class ProductionBehavior : public StorageBehavior {
    public:
        virtual void Update(GridCell* cell) override;
        virtual int ReceiveMessage(GridCell* cell, const CellMessage& message) override;
        virtual std::vector<CellMessage> GetMessageList() const override;

    protected:
        // Child classes define their production rules here. 
        // Return an empty map for inputs if the building produces unconditionally (e.g., Miners).
        // Map format: { ItemTypeID : AmountPerUpdate }
        virtual std::map<int, int> GetRecipeInputs(const GridCell* cell) const = 0;
        virtual std::pair<int, int> GetRecipeOutput(const GridCell* cell) const = 0;

        float GetMaxStorage(GridCell* cell);
    };
}