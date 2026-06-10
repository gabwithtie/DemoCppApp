#pragma once

#include "ViewportTool.h"

namespace picsel {

    class BrushTool : public ViewportTool {
    public:
        std::string GetName() const override { return "Brush / Eraser"; }

        void DrawSettings() override;
        void ProcessInteraction(std::string asset_id, ImVec2 canvas_min, ImVec2 canvas_max, float zoom) override;

    private:
        // Tool State
        float m_radius = 2;
        float m_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA

        // Helper to convert screen coordinates to local pixel coordinates
        void ScreenToPixel(ImVec2 screen_pos, ImVec2 canvas_min, float zoom, int& out_px, int& out_py);
    };
}