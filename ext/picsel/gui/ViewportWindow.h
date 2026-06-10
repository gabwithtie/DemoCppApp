#pragma once

#include "gui/main/GuiWindow.h"

namespace picsel {

    class ViewportWindow : public app::GuiWindow {
    public:
        ViewportWindow() = default;
        ~ViewportWindow() = default;

        std::string GetWindowId() override { return "Canvas Viewport##Picsel"; }

    protected:
        void DrawSelf() override;
    };
}