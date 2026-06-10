#pragma once

#include "gui/main/GuiWindow.h"
#include "picsel/animation/AnimationClip.h"
#include "tools/ViewportTool.h"

#include <vector>
#include <memory>
#include <string>

namespace picsel {

    // ---------------------------------------------------------------------------
    // AnimationWindow
    //
    // Layout:
    //   [ Center ]  Composite preview — renders all visible layers for the
    //               current frame with pan/zoom. Drawing tools operate here.
    //   [ Bottom ]  Timeline grid — layers as rows (top = highest Z),
    //               frames as columns. Click to scrub; drag layers to reorder.
    //
    // The window holds one active AnimationClip at a time.
    // Clips are created externally and injected via SetClip().
    // ---------------------------------------------------------------------------
    class AnimationWindow : public app::GuiWindow {
    public:
        AnimationWindow();
        ~AnimationWindow() = default;

        std::string GetWindowId() override { return "Animation##Picsel"; }

        // Call once per frame from the app loop to advance playback.
        void Tick(float delta_seconds);

    protected:
        void DrawSelf() override;

    private:
        // --- Sub-sections ---
        void DrawToolbar();
        void DrawPreview();
        void DrawTimeline();

        // --- Preview helpers ---
        void DrawCompositeFrame(ImDrawList* dl, int frame_idx,
                                ImVec2 canvas_min, ImVec2 canvas_max, float zoom);

        // --- Timeline helpers ---
        void DrawTimelineLayerColumn(float col_width, float row_height);
        void DrawTimelineFrameGrid(ImVec2 origin, float col_width, float row_height);

        // --- Drag-reorder state ---
        int  m_drag_layer_from = -1;
        int  m_drag_layer_to   = -1;

        // --- Preview pan/zoom state ---
        float m_preview_pan_x = 0.0f;
        float m_preview_pan_y = 0.0f;
        float m_preview_zoom  = 1.0f;
        bool  m_needs_view_reset = true;

        // --- Tool registry (same ViewportTool interface as ViewportWindow) ---
        std::vector<std::unique_ptr<ViewportTool>> m_tools;
        int m_active_tool_idx = 0;

        // --- Data ---
        AnimationClip* m_clip = nullptr;

        // Timeline scroll
        float m_timeline_scroll_x = 0.0f;

        // Constants
        static constexpr float k_timeline_height  = 180.0f;
        static constexpr float k_layer_col_width  = 160.0f;
        static constexpr float k_frame_col_width  =  36.0f;
        static constexpr float k_row_height       =  28.0f;
    };

} // namespace picsel
