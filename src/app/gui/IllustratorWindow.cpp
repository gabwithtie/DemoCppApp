#include "IllustratorWindow.hpp"

#include "../App.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <imgui.h>

namespace {

static ImU32 ToImU32(uint32_t rgba) {
    const uint8_t r = static_cast<uint8_t>((rgba >> 24) & 0xFFu);
    const uint8_t g = static_cast<uint8_t>((rgba >> 16) & 0xFFu);
    const uint8_t b = static_cast<uint8_t>((rgba >> 8) & 0xFFu);
    const uint8_t a = static_cast<uint8_t>(rgba & 0xFFu);

    return IM_COL32(r, g, b, a);
}

static uint32_t AlphaCompositeOver(uint32_t dst, uint32_t src) {
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

} // namespace

namespace app {

IllustratorWindow::IllustratorWindow(Tool* initialTool) : activeTool(initialTool) {}

void IllustratorWindow::SetActiveTool(Tool* tool) {
    activeTool = tool;
}

void IllustratorWindow::ResetView(const ImVec2& canvasSize, int imageWidth, int imageHeight) {
    zoom = std::max(0.01f, std::min(canvasSize.x / static_cast<float>(imageWidth), canvasSize.y / static_cast<float>(imageHeight)));
    panOffset = ImVec2((canvasSize.x - static_cast<float>(imageWidth) * zoom) * 0.5f,
                       (canvasSize.y - static_cast<float>(imageHeight) * zoom) * 0.5f);
}

void IllustratorWindow::DrawSelf() {
    App& app = App::GetInstance();
    Model::Project& project = app.project;

    if (project.w <= 0 || project.h <= 0) {
        ImGui::TextUnformatted("Project dimensions are invalid.");
        return;
    }

    for (auto& layer : project.layers) {
        layer.EnsureSize(project.w, project.h, Model::Layer::Transparent);
    }

    ImGui::Text("Canvas: %d x %d", project.w, project.h);
    ImGui::SameLine();
    ImGui::Text("Zoom: %.0f%%", zoom * 100.0f);
    ImGui::SameLine();
    if (ImGui::Button("Reset View")) {
        lastProjectWidth = -1;
    }

    ImGui::Separator();

    const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x <= 4.0f || canvasSize.y <= 4.0f) {
        return;
    }

    if (lastProjectWidth != project.w || lastProjectHeight != project.h || lastProjectWidth < 0) {
        ResetView(canvasSize, project.w, project.h);
        lastProjectWidth = project.w;
        lastProjectHeight = project.h;
    }

    const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
    const ImVec2 canvasMax = ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + canvasSize.y);

    ImGui::InvisibleButton("ViewerCanvas", canvasSize, ImGuiButtonFlags_MouseButtonMiddle);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    ImGuiIO& io = ImGui::GetIO();

    if (hovered && io.MouseWheel != 0.0f) {
        const float oldZoom = zoom;
        const float zoomFactor = std::pow(1.1f, io.MouseWheel);
        zoom = std::clamp(zoom * zoomFactor, 0.05f, 64.0f);

        const ImVec2 imageOriginBefore = ImVec2(canvasOrigin.x + panOffset.x, canvasOrigin.y + panOffset.y);
        const ImVec2 worldPos = ImVec2((io.MousePos.x - imageOriginBefore.x) / oldZoom,
                                       (io.MousePos.y - imageOriginBefore.y) / oldZoom);

        panOffset = ImVec2(io.MousePos.x - canvasOrigin.x - worldPos.x * zoom,
                           io.MousePos.y - canvasOrigin.y - worldPos.y * zoom);
    }

    if ((hovered || active) && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
        panOffset.x += io.MouseDelta.x;
        panOffset.y += io.MouseDelta.y;
    }

    Model::Layer* selectedLayer = nullptr;
    if (!project.layers.empty()) {
        app.selected_layer_index = std::clamp(app.selected_layer_index, 0, static_cast<int>(project.layers.size()) - 1);
        selectedLayer = &project.layers[static_cast<size_t>(app.selected_layer_index)];
    }

    Tool::CanvasContext toolCtx{
        .project = project,
        .selectedLayer = selectedLayer,
        .selectedLayerIndex = app.selected_layer_index,
        .canvasOrigin = canvasOrigin,
        .canvasMax = canvasMax,
        .imageOrigin = ImVec2(canvasOrigin.x + panOffset.x, canvasOrigin.y + panOffset.y),
        .zoom = zoom,
        .hovered = hovered,
        .active = active
    };

    bool toolAbsorbedInput = false;
    if (activeTool != nullptr) {
        toolAbsorbedInput = activeTool->ProcessCanvasInput(toolCtx);
    }

    if (toolAbsorbedInput) {
        for (auto& layer : project.layers) {
            layer.EnsureSize(project.w, project.h, Model::Layer::Transparent);
        }
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(canvasOrigin, canvasMax, true);

    const int checkerSize = 16;
    for (int y = 0; y < static_cast<int>(canvasSize.y); y += checkerSize) {
        for (int x = 0; x < static_cast<int>(canvasSize.x); x += checkerSize) {
            const bool light = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            const ImU32 color = light ? IM_COL32(70, 70, 70, 255) : IM_COL32(55, 55, 55, 255);
            drawList->AddRectFilled(ImVec2(canvasOrigin.x + static_cast<float>(x), canvasOrigin.y + static_cast<float>(y)),
                                    ImVec2(canvasOrigin.x + static_cast<float>(std::min(x + checkerSize, static_cast<int>(canvasSize.x))),
                                           canvasOrigin.y + static_cast<float>(std::min(y + checkerSize, static_cast<int>(canvasSize.y)))),
                                    color);
        }
    }

    const ImVec2 imageOrigin = toolCtx.imageOrigin;
    const int step = std::max(1, static_cast<int>(std::ceil(1.0f / zoom)));

    int startX = static_cast<int>(std::floor((canvasOrigin.x - imageOrigin.x) / zoom));
    int startY = static_cast<int>(std::floor((canvasOrigin.y - imageOrigin.y) / zoom));
    int endX = static_cast<int>(std::ceil((canvasMax.x - imageOrigin.x) / zoom));
    int endY = static_cast<int>(std::ceil((canvasMax.y - imageOrigin.y) / zoom));

    startX = std::clamp(startX, 0, project.w);
    startY = std::clamp(startY, 0, project.h);
    endX = std::clamp(endX, 0, project.w);
    endY = std::clamp(endY, 0, project.h);

    for (int y = startY; y < endY; y += step) {
        for (int x = startX; x < endX; x += step) {
            uint32_t composited = Model::Layer::Transparent;
            for (const auto& layer : project.layers) {
                if (!layer.visible) {
                    continue;
                }
                composited = AlphaCompositeOver(composited, layer.GetPixel(x, y));
            }

            if ((composited & 0xFFu) == 0u) {
                continue;
            }

            const ImVec2 p0 = ImVec2(imageOrigin.x + static_cast<float>(x) * zoom,
                                     imageOrigin.y + static_cast<float>(y) * zoom);
            const ImVec2 p1 = ImVec2(imageOrigin.x + static_cast<float>(std::min(x + step, project.w)) * zoom,
                                     imageOrigin.y + static_cast<float>(std::min(y + step, project.h)) * zoom);
            drawList->AddRectFilled(p0, p1, ToImU32(composited));
        }
    }

    drawList->AddRect(imageOrigin,
                      ImVec2(imageOrigin.x + static_cast<float>(project.w) * zoom,
                             imageOrigin.y + static_cast<float>(project.h) * zoom),
                      IM_COL32(220, 220, 220, 255));

    if (activeTool != nullptr) {
        activeTool->DrawOverlay(toolCtx, drawList);
    }

    drawList->PopClipRect();
}

} // namespace app
