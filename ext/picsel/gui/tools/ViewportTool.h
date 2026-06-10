#pragma once

#include <imgui.h>
#include <string>

namespace picsel {

    class Viewport;
    class AnimationManager;

    class ViewportTool {
    public:
        virtual ~ViewportTool() = default;

        // The name displayed in the toolbar
        virtual std::string GetName() const = 0;

        // Called by ViewportWindow to draw tool-specific settings (e.g., in a side panel)
        virtual void DrawSettings() = 0;

        // Called by ViewportWindow when the tool is active and the mouse is over the canvas
        // canvas_min/max are the screen-space bounds of the drawn canvas.
        virtual void ProcessInteraction(Viewport* vp, ImVec2 canvas_min, ImVec2 canvas_max, float zoom) = 0;
        virtual void ProcessInteractionAnim(AnimationManager* vp, ImVec2 canvas_min, ImVec2 canvas_max, float zoom) = 0;
    };
}