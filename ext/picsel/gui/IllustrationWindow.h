#pragma once

#include "gui/main/GuiWindow.h"
#include "tools/ViewportTool.h"
#include <vector>
#include <memory>
#include <string>

namespace picsel {

    class IllustrationWindow : public app::GuiWindow {
    public:
        IllustrationWindow();
        ~IllustrationWindow();

        std::string GetWindowId() override { return "Canvas Viewport##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        float m_pan_x = 0.0f;
        float m_pan_y = 0.0f;
        float m_zoom = 1.0f;
        bool needs_view_reset = true;
        std::string activeCanvas;

        // Tool Registry
        std::vector<std::unique_ptr<ViewportTool>> m_tools;
        int m_active_tool_idx = 0;
    };

} // namespace picsel