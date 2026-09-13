#include "PencilTool.hpp"
#include "../../App.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace app {

PencilTool::PencilTool() : Tool("Pencil Tool") {}

uint32_t PencilTool::PackRgba8(const ImVec4& color) {
    const uint8_t r = static_cast<uint8_t>(std::clamp(color.x, 0.0f, 1.0f) * 255.0f);
    const uint8_t g = static_cast<uint8_t>(std::clamp(color.y, 0.0f, 1.0f) * 255.0f);
    const uint8_t b = static_cast<uint8_t>(std::clamp(color.z, 0.0f, 1.0f) * 255.0f);
    const uint8_t a = static_cast<uint8_t>(std::clamp(color.w, 0.0f, 1.0f) * 255.0f);

    return (static_cast<uint32_t>(r) << 24) |
           (static_cast<uint32_t>(g) << 16) |
           (static_cast<uint32_t>(b) << 8) |
           static_cast<uint32_t>(a);
}

uint32_t PencilTool::AlphaBlendOver(uint32_t dst, uint32_t src) {
    const float sr = static_cast<float>((src >> 24) & 0xFFu) / 255.0f;
    const float sg = static_cast<float>((src >> 16) & 0xFFu) / 255.0f;
    const float sb = static_cast<float>((src >> 8) & 0xFFu) / 255.0f;
    const float sa = static_cast<float>(src & 0xFFu) / 255.0f;

    const float dr = static_cast<float>((dst >> 24) & 0xFFu) / 255.0f;
    const float dg = static_cast<float>((dst >> 16) & 0xFFu) / 255.0f;
    const float db = static_cast<float>((dst >> 8) & 0xFFu) / 255.0f;
    const float da = static_cast<float>(dst & 0xFFu) / 255.0f;

    const float outA = sa + da * (1.0f - sa);
    if (outA <= 0.0f) {
        return Model::Layer::Transparent;
    }

    const float outR = (sr * sa + dr * da * (1.0f - sa)) / outA;
    const float outG = (sg * sa + dg * da * (1.0f - sa)) / outA;
    const float outB = (sb * sa + db * da * (1.0f - sa)) / outA;

    const uint8_t r = static_cast<uint8_t>(std::clamp(outR, 0.0f, 1.0f) * 255.0f);
    const uint8_t g = static_cast<uint8_t>(std::clamp(outG, 0.0f, 1.0f) * 255.0f);
    const uint8_t b = static_cast<uint8_t>(std::clamp(outB, 0.0f, 1.0f) * 255.0f);
    const uint8_t a = static_cast<uint8_t>(std::clamp(outA, 0.0f, 1.0f) * 255.0f);

    return (static_cast<uint32_t>(r) << 24) |
           (static_cast<uint32_t>(g) << 16) |
           (static_cast<uint32_t>(b) << 8) |
           static_cast<uint32_t>(a);
}

void PencilTool::PaintAtPixel(Model::Layer& layer, int cx, int cy, int canvasW, int canvasH, bool erase) {
    const int clampedRadius = std::max(1, radius);
    const int rr = clampedRadius * clampedRadius;
    const uint32_t brush = PackRgba8(brushColor);

    for (int y = cy - clampedRadius; y <= cy + clampedRadius; ++y) {
        for (int x = cx - clampedRadius; x <= cx + clampedRadius; ++x) {
            const int dx = x - cx;
            const int dy = y - cy;
            if (dx * dx + dy * dy > rr) {
                continue;
            }
            if (x < 0 || y < 0 || x >= canvasW || y >= canvasH) {
                continue;
            }

            if (erase) {
                layer.SetPixel(x, y, Model::Layer::Transparent);
                continue;
            }

            if (mode == PaintMode::Override) {
                layer.SetPixel(x, y, brush);
            } else {
                const uint32_t dst = layer.GetPixel(x, y);
                layer.SetPixel(x, y, AlphaBlendOver(dst, brush));
            }
        }
    }
}

bool PencilTool::ProcessCanvasInput(const CanvasContext& ctx) {
    App& app = App::GetInstance();

    if (strokeBatchOpen && !ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        app.edit_history.EndBatch(app.project);
        strokeBatchOpen = false;
    }

    if (!ctx.hovered || ctx.selectedLayer == nullptr || ctx.selectedLayerIndex < 0) {
        return false;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        return false;
    }

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const int px = static_cast<int>(std::floor((mouse.x - ctx.imageOrigin.x) / ctx.zoom));
    const int py = static_cast<int>(std::floor((mouse.y - ctx.imageOrigin.y) / ctx.zoom));

    if (px < 0 || py < 0 || px >= ctx.project.w || py >= ctx.project.h) {
        return false;
    }

    const bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    const bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (!leftDown && !rightDown) {
        return false;
    }

    if (!strokeBatchOpen) {
        app.edit_history.BeginBatch(app.project, "Pencil Stroke");
        strokeBatchOpen = true;
    }

    if (leftDown) {
        const int layerIndex = ctx.selectedLayerIndex;
        app.edit_history.ApplyInBatch(app.project, "Pencil Stroke", [&](Model::Project& project) {
            if (layerIndex >= 0 && layerIndex < static_cast<int>(project.layers.size())) {
                PaintAtPixel(project.layers[static_cast<size_t>(layerIndex)], px, py, project.w, project.h, false);
            }
        });
        return true;
    }

    if (rightDown) {
        const int layerIndex = ctx.selectedLayerIndex;
        app.edit_history.ApplyInBatch(app.project, "Pencil Stroke", [&](Model::Project& project) {
            if (layerIndex >= 0 && layerIndex < static_cast<int>(project.layers.size())) {
                PaintAtPixel(project.layers[static_cast<size_t>(layerIndex)], px, py, project.w, project.h, true);
            }
        });
        return true;
    }

    return false;
}

void PencilTool::DrawBrushOverlay(const CanvasContext& ctx, ImDrawList* drawList) const {
    if (!ctx.hovered || ctx.selectedLayer == nullptr) {
        return;
    }

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const int cx = static_cast<int>(std::floor((mouse.x - ctx.imageOrigin.x) / ctx.zoom));
    const int cy = static_cast<int>(std::floor((mouse.y - ctx.imageOrigin.y) / ctx.zoom));

    if (cx < 0 || cy < 0 || cx >= ctx.project.w || cy >= ctx.project.h) {
        return;
    }

    const int clampedRadius = std::max(1, radius);
    const int rr = clampedRadius * clampedRadius;

    const uint32_t brush = PackRgba8(brushColor);
    const ImU32 fillColor = IM_COL32((brush >> 24) & 0xFFu, (brush >> 16) & 0xFFu, (brush >> 8) & 0xFFu, 65);
    const ImU32 strokeColor = IM_COL32(255, 255, 255, 190);

    for (int y = cy - clampedRadius; y <= cy + clampedRadius; ++y) {
        for (int x = cx - clampedRadius; x <= cx + clampedRadius; ++x) {
            const int dx = x - cx;
            const int dy = y - cy;
            if (dx * dx + dy * dy > rr) {
                continue;
            }
            if (x < 0 || y < 0 || x >= ctx.project.w || y >= ctx.project.h) {
                continue;
            }

            const ImVec2 p0 = ImVec2(ctx.imageOrigin.x + static_cast<float>(x) * ctx.zoom,
                                     ctx.imageOrigin.y + static_cast<float>(y) * ctx.zoom);
            const ImVec2 p1 = ImVec2(p0.x + ctx.zoom, p0.y + ctx.zoom);

            drawList->AddRectFilled(p0, p1, fillColor);
            drawList->AddRect(p0, p1, strokeColor);
        }
    }
}

void PencilTool::DrawOverlay(const CanvasContext& ctx, ImDrawList* drawList) {
    DrawBrushOverlay(ctx, drawList);
}

void PencilTool::DrawSettingsUi() {
    ImGui::TextUnformatted("Pencil Tool Settings");
    ImGui::Text("Active: %s", GetToolName().c_str());
    ImGui::SliderInt("Radius", &radius, 1, 64);
    ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&brushColor));

    int modeSelection = mode == PaintMode::AlphaBlend ? 0 : 1;
    ImGui::RadioButton("Alpha Blend", &modeSelection, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Override", &modeSelection, 1);
    mode = modeSelection == 0 ? PaintMode::AlphaBlend : PaintMode::Override;

    ImGui::Separator();
    ImGui::TextUnformatted("Left click paints, right click erases.");
}

} // namespace app
