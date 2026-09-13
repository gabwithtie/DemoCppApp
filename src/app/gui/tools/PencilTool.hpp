#pragma once

#include "Tool.hpp"
#include "ToolRegistry.hpp"

#include <imgui.h>

namespace app {

class PencilTool : public Tool {
public:
    enum class PaintMode {
        AlphaBlend,
        Override
    };

    PencilTool();

    bool ProcessCanvasInput(const CanvasContext& ctx) override;
    void DrawOverlay(const CanvasContext& ctx, ImDrawList* drawList) override;
    void DrawSettingsUi() override;

private:
    int radius{2};
    ImVec4 brushColor{1.0f, 1.0f, 1.0f, 1.0f};
    PaintMode mode{PaintMode::AlphaBlend};
    bool strokeBatchOpen{false};

    static uint32_t PackRgba8(const ImVec4& color);
    static uint32_t AlphaBlendOver(uint32_t dst, uint32_t src);

    void PaintAtPixel(Model::Layer& layer, int cx, int cy, int canvasW, int canvasH, bool erase);
    void DrawBrushOverlay(const CanvasContext& ctx, ImDrawList* drawList) const;
};

GBE_REGISTER_TOOL(PencilTool);

} // namespace app
