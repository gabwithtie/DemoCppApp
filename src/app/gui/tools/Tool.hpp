#pragma once

#include "../../model/Project.hpp"

#include <imgui.h>
#include <string>

namespace app {

class Tool {
public:
    struct CanvasContext {
        Model::Project& project;
        Model::Layer* selectedLayer;
        int selectedLayerIndex;

        ImVec2 canvasOrigin;
        ImVec2 canvasMax;
        ImVec2 imageOrigin;
        float zoom;

        bool hovered;
        bool active;
    };

    explicit Tool(const char* toolName) : name(toolName) {}
    virtual ~Tool() = default;

    const std::string& GetToolName() const { return name; }

    virtual bool ProcessCanvasInput(const CanvasContext& ctx) = 0;
    virtual void DrawOverlay(const CanvasContext& ctx, ImDrawList* drawList) = 0;
    virtual void DrawSettingsUi() = 0;

private:
    std::string name;
};

} // namespace app
