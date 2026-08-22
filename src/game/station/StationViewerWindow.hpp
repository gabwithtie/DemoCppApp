#pragma once

#include "../gui/main/GuiWindow.h"
#include "GameSimulation.hpp"

namespace app {

    struct PendingCargoSale {
        std::string train_id;
        size_t carriage_index = 0;
        std::string station_id;
        std::string item_id;
        double unit_price = 0.0;
        int max_quantity = 0;
        int selected_quantity = 1;
        bool open_modal = false;
        bool same_station_valid = true;
    };

    class StationViewerWindow : public GuiWindow {
    public:
        StationViewerWindow(GameSimulation& simulation);
        std::string GetWindowId() override { return "Station Inspector##app"; }

    protected:
        void DrawSelf() override;

    private:
        GameSimulation& m_sim;
        PendingCargoSale m_pending_sale;
    };

} // namespace app