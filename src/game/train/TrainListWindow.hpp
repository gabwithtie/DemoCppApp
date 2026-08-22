#pragma once

#include "../gui/main/GuiWindow.h"
#include "../GameSimulation.hpp"

namespace app {

    struct PendingCargoPurchase {
        std::string station_id;
        std::string train_id;
        size_t carriage_index = 0;
        std::string item_id;
        double unit_price = 0.0;
        int max_quantity = 0;
        int selected_quantity = 1;
        bool open_modal = false;
        bool same_station_valid = true;
    };

    class TrainListWindow : public GuiWindow {
    public:
        TrainListWindow(GameSimulation& simulation);
        std::string GetWindowId() override { return "Train Controls##app"; }

    protected:
        void DrawSelf() override;

    private:
        GameSimulation& m_sim;
        PendingCargoPurchase m_pending_purchase;
    };

} // namespace app