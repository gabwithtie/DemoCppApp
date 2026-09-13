#pragma once

#include "../../gui/main/GuiWindow.h"
#include "tools/Tool.hpp"

#include <imgui.h>

namespace app {

class IllustratorWindow : public app::GuiWindow {
public:
    explicit IllustratorWindow(Tool* initialTool = nullptr);

    std::string GetWindowId() override { return "Illustrator"; }
    void SetActiveTool(Tool* tool);
    Tool* GetActiveTool() const { return activeTool; }

protected:
    void DrawSelf() override;

private:
    float zoom{1.0f};
    ImVec2 panOffset{0.0f, 0.0f};
    int lastProjectWidth{-1};
    int lastProjectHeight{-1};
    Tool* activeTool{nullptr};

    void ResetView(const ImVec2& canvasSize, int imageWidth, int imageHeight);
};

} // namespace app
