#pragma once

#include "gui/main/GuiWindow.h"
#include "picsel/animation/AnimationClip.h"
#include "picsel/gui/tools/ViewportTool.h"

#include <vector>
#include <memory>
#include <string>

namespace picsel {

    class AnimationWindow : public app::GuiWindow {
    public:
        AnimationWindow();
        ~AnimationWindow() = default;

        std::string GetWindowId() override { return "Animation##Picsel"; }

        // Core tick method called from the main application thread to advance frames
        void Tick(float delta_seconds);

        // Grant sub-component architectures access to window state vectors
        friend class AnimationToolbar;
        friend class AnimationViewport;
        friend class AnimationTimeline;

    protected:
        void DrawSelf() override;

    private:
        // --- Drag-and-Drop layer state ---
        int  m_drag_layer_from = -1;
        int  m_drag_layer_to = -1;

        // --- Viewport pan/zoom state ---
        float m_preview_pan_x = 0.0f;
        float m_preview_pan_y = 0.0f;
        float m_preview_zoom = 1.0f;
        bool  m_needs_view_reset = true;

        // --- Interactive workspace tool registry ---
        std::vector<std::unique_ptr<ViewportTool>> m_tools;
        int m_active_tool_idx = 0;

        // --- Time tracking accumulator for playback calculations ---
        float m_playback_acc = 0.0f;

        // --- Horizon scrolling parameters ---
        float m_timeline_scroll_x = 0.0f;

        // --- Fixed UI spacing constants ---
        static constexpr float k_timeline_height = 180.0f;
        static constexpr float k_layer_col_width = 150.0f;
        static constexpr float k_row_height = 24.0f;
        static constexpr float k_col_width = 20.0f;
    };
}