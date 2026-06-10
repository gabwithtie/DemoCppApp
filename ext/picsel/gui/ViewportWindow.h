#pragma once

#include "gui/main/GuiWindow.h"
#include "tools/ViewportTool.h"
#include <vector>
#include <memory>
#include <string>

namespace picsel {

    class ViewportWindow : public app::GuiWindow {
    public:
        ViewportWindow();
        ~ViewportWindow(); // NEW: Override destructor for save-on-close

        std::string GetWindowId() override { return "Canvas Viewport##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        void SaveCanvas(const std::string& asset_id);

        // Tool Registry
        std::vector<std::unique_ptr<ViewportTool>> m_tools;
        int m_active_tool_idx = 0;

        // Auto-save tracking
        std::string m_last_active_asset_id = "";
    };
}