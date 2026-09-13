#pragma once

#include "Tool.hpp"
#include "ToolRegistry.hpp"

#include <imgui.h>
#include <vector>

namespace app {

class PathDrawTool : public Tool {
public:
    enum class SmoothingMode {
        Standard,
        Spherical
    };

    PathDrawTool();

    bool ProcessCanvasInput(const CanvasContext& ctx) override;
    void DrawOverlay(const CanvasContext& ctx, ImDrawList* drawList) override;
    void DrawSettingsUi() override;

private:
    std::vector<ImVec2> points;
    int draggingPointIndex{-1};

    ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float thicknessStart{2.0f};
    float thicknessMiddle{5.0f};
    float thicknessEnd{2.0f};
    float thickestPosition{0.5f};
    SmoothingMode smoothingMode{SmoothingMode::Standard};

    static uint32_t PackRgba8(const ImVec4& inColor);
    static uint32_t AlphaBlendOver(uint32_t dst, uint32_t src);

    float EvaluateThickness(float t) const;
    int FindHoveredPoint(const CanvasContext& ctx, float* outDistancePixels = nullptr) const;
    ImVec2 MouseToImage(const CanvasContext& ctx) const;

    static float Distance(const ImVec2& a, const ImVec2& b);
    static ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t);
    static ImVec2 CatmullRom(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t);
    static std::vector<ImVec2> SampleCircularArc(const ImVec2& start, const ImVec2& mid, const ImVec2& end, int samplesPerArc);
    static std::vector<ImVec2> ResamplePolyline(const std::vector<ImVec2>& polyline, int targetCount);

    std::vector<ImVec2> BuildSampledPath() const;
    std::vector<ImVec2> BuildSampledPathStandard() const;
    std::vector<ImVec2> BuildSampledPathSpherical() const;
    void PaintDisc(Model::Layer& layer, const ImVec2& center, float radius, uint32_t packedColor);
    void CommitToLayer(const CanvasContext& ctx);
};

GBE_REGISTER_TOOL(PathDrawTool);

} // namespace app
