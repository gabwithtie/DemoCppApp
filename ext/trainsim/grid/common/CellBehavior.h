#pragma once
#include "GridCell.h"
#include "CellMessage.h"

#include <functional>

namespace Trainsim {
    class CellBehavior {
    public:
        virtual ~CellBehavior() = default;

        virtual void Update(GridCell* cell) = 0;
        virtual int ReceiveMessage(GridCell* cell, const CellMessage& message) = 0;

        // Returns a list of compatible message types this behavior can interpret
        virtual std::vector<CellMessage> GetMessageList() const = 0;

        //Utility Functions
        static void DoOnAdjacent(GridCell* center, std::function<void(int nx, int ny)> func);
        static void DoOnRadius(GridCell* center, float radius, std::function<void(int nx, int ny)> func);
    };
}