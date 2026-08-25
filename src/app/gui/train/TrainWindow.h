#pragma once

#include "../gui/main/GuiWindow.h"

namespace gsr {
    class TrainWindow : public app::GuiWindow {
    private:
        float zoom_level = 1.0f;
        int carriage_count = 8;

        void HandleZoom();
        void HandleDragging();
        void DrawCarriages();

    protected:
        void DrawSelf() override;

    public:
        std::string GetWindowId() override { return "Train Simulator"; }
    };
}