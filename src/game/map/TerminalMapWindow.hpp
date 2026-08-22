#pragma once

#include "../gui/main/GuiWindow.h"
#include "../GameSimulation.hpp"

#include <imgui.h>

namespace app {

    class TerminalMapWindow : public GuiWindow {
    public:
        TerminalMapWindow(GameSimulation& simulation);
        std::string GetWindowId() override { return "Terminal Map##app"; }

    protected:
        void DrawSelf() override;

    private:
        GameSimulation& m_sim;
        gbe::Vector2 m_pan{ 0.0f, 0.0f };
        float m_zoom = 1.0f;

        ImVec2 WorldToScreen(const gbe::Vector2& world_pos, const ImVec2& canvas_origin) const;
    };

} // namespace app