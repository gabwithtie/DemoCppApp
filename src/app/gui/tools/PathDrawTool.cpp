#include "PathDrawTool.hpp"
#include "../../App.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace app {

PathDrawTool::PathDrawTool() : Tool("Path Draw Tool") {}

uint32_t PathDrawTool::PackRgba8(const ImVec4& inColor) {
    const uint8_t r = static_cast<uint8_t>(std::clamp(inColor.x, 0.0f, 1.0f) * 255.0f);
    const uint8_t g = static_cast<uint8_t>(std::clamp(inColor.y, 0.0f, 1.0f) * 255.0f);
    const uint8_t b = static_cast<uint8_t>(std::clamp(inColor.z, 0.0f, 1.0f) * 255.0f);
    const uint8_t a = static_cast<uint8_t>(std::clamp(inColor.w, 0.0f, 1.0f) * 255.0f);

    return (static_cast<uint32_t>(r) << 24) |
           (static_cast<uint32_t>(g) << 16) |
           (static_cast<uint32_t>(b) << 8) |
           static_cast<uint32_t>(a);
}

uint32_t PathDrawTool::AlphaBlendOver(uint32_t dst, uint32_t src) {
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

float PathDrawTool::Distance(const ImVec2& a, const ImVec2& b) {
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

ImVec2 PathDrawTool::Lerp(const ImVec2& a, const ImVec2& b, float t) {
    return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

ImVec2 PathDrawTool::CatmullRom(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;

    return ImVec2(
        0.5f * ((2.0f * p1.x) +
                (-p0.x + p2.x) * t +
                (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
        0.5f * ((2.0f * p1.y) +
                (-p0.y + p2.y) * t +
                (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3)
    );
}

std::vector<ImVec2> PathDrawTool::SampleCircularArc(const ImVec2& start, const ImVec2& mid, const ImVec2& end, int samplesPerArc) {
    constexpr float kEpsilon = 1e-6f;
    constexpr float kTwoPi = 6.28318530718f;

    auto normalizeAngle = [](float a) {
        while (a < 0.0f) {
            a += kTwoPi;
        }
        while (a >= kTwoPi) {
            a -= kTwoPi;
        }
        return a;
    };

    auto deltaCcw = [&](float from, float to) {
        const float f = normalizeAngle(from);
        const float t = normalizeAngle(to);
        float d = t - f;
        if (d < 0.0f) {
            d += kTwoPi;
        }
        return d;
    };

    const int sampleCount = std::max(2, samplesPerArc);
    std::vector<ImVec2> out;
    out.reserve(static_cast<size_t>(sampleCount) + 1);

    const float ax = start.x;
    const float ay = start.y;
    const float bx = mid.x;
    const float by = mid.y;
    const float cx = end.x;
    const float cy = end.y;

    const float d = 2.0f * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    if (std::abs(d) < kEpsilon) {
        for (int i = 0; i <= sampleCount; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(sampleCount);
            out.push_back(Lerp(start, end, t));
        }
        return out;
    }

    const float a2 = ax * ax + ay * ay;
    const float b2 = bx * bx + by * by;
    const float c2 = cx * cx + cy * cy;

    const float ux = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
    const float uy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
    const ImVec2 center(ux, uy);

    const float radius = Distance(center, start);
    if (radius < kEpsilon) {
        out.push_back(start);
        out.push_back(end);
        return out;
    }

    const float angleStart = std::atan2(start.y - center.y, start.x - center.x);
    const float angleMid = std::atan2(mid.y - center.y, mid.x - center.x);
    const float angleEnd = std::atan2(end.y - center.y, end.x - center.x);

    const float deltaEndCcw = deltaCcw(angleStart, angleEnd);
    const float deltaMidCcw = deltaCcw(angleStart, angleMid);
    const bool useCcw = deltaMidCcw <= deltaEndCcw;
    const float sweep = useCcw ? deltaEndCcw : -(kTwoPi - deltaEndCcw);

    for (int i = 0; i <= sampleCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleCount);
        const float a = angleStart + sweep * t;
        out.emplace_back(center.x + std::cos(a) * radius, center.y + std::sin(a) * radius);
    }

    return out;
}

std::vector<ImVec2> PathDrawTool::ResamplePolyline(const std::vector<ImVec2>& polyline, int targetCount) {
    std::vector<ImVec2> out;
    if (polyline.empty() || targetCount <= 0) {
        return out;
    }

    if (polyline.size() == 1 || targetCount == 1) {
        out.push_back(polyline.front());
        return out;
    }

    out.reserve(static_cast<size_t>(targetCount));

    std::vector<float> cumulative(polyline.size(), 0.0f);
    for (size_t i = 1; i < polyline.size(); ++i) {
        cumulative[i] = cumulative[i - 1] + Distance(polyline[i - 1], polyline[i]);
    }

    const float total = cumulative.back();
    if (total <= 1e-6f) {
        for (int i = 0; i < targetCount; ++i) {
            out.push_back(polyline.front());
        }
        return out;
    }

    size_t seg = 1;
    for (int i = 0; i < targetCount; ++i) {
        const float t = targetCount == 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(targetCount - 1);
        const float wanted = t * total;

        while (seg < cumulative.size() && cumulative[seg] < wanted) {
            ++seg;
        }

        if (seg >= cumulative.size()) {
            out.push_back(polyline.back());
            continue;
        }

        const float segStart = cumulative[seg - 1];
        const float segLen = std::max(1e-6f, cumulative[seg] - segStart);
        const float lt = (wanted - segStart) / segLen;
        out.push_back(Lerp(polyline[seg - 1], polyline[seg], lt));
    }

    return out;
}

ImVec2 PathDrawTool::MouseToImage(const CanvasContext& ctx) const {
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    return ImVec2(
        (mouse.x - ctx.imageOrigin.x) / ctx.zoom,
        (mouse.y - ctx.imageOrigin.y) / ctx.zoom
    );
}

float PathDrawTool::EvaluateThickness(float t) const {
    const float p = std::clamp(thickestPosition, 0.0f, 1.0f);
    const float start = std::max(0.1f, thicknessStart);
    const float mid = std::max(0.1f, thicknessMiddle);
    const float end = std::max(0.1f, thicknessEnd);

    if (p <= 0.0001f) {
        return start + (end - start) * t;
    }
    if (p >= 0.9999f) {
        return start + (end - start) * t;
    }

    if (t <= p) {
        const float lt = t / p;
        return start + (mid - start) * lt;
    }

    const float lt = (t - p) / (1.0f - p);
    return mid + (end - mid) * lt;
}

int PathDrawTool::FindHoveredPoint(const CanvasContext& ctx, float* outDistancePixels) const {
    if (points.empty()) {
        if (outDistancePixels) {
            *outDistancePixels = 0.0f;
        }
        return -1;
    }

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const float hoverThreshold = 10.0f;

    int bestIndex = -1;
    float bestDistance = hoverThreshold;

    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        const ImVec2 sp = ImVec2(
            ctx.imageOrigin.x + points[static_cast<size_t>(i)].x * ctx.zoom,
            ctx.imageOrigin.y + points[static_cast<size_t>(i)].y * ctx.zoom
        );
        const float d = Distance(mouse, sp);
        if (d <= bestDistance) {
            bestDistance = d;
            bestIndex = i;
        }
    }

    if (outDistancePixels) {
        *outDistancePixels = bestDistance;
    }

    return bestIndex;
}

std::vector<ImVec2> PathDrawTool::BuildSampledPathStandard() const {
    std::vector<ImVec2> sampled;
    if (points.size() < 2) {
        return sampled;
    }

    if (points.size() == 2) {
        sampled.push_back(points[0]);
        sampled.push_back(points[1]);
        return sampled;
    }

    constexpr int samplesPerSegment = 24;
    sampled.reserve((points.size() - 1) * samplesPerSegment + 1);

    for (int i = 0; i < static_cast<int>(points.size()) - 1; ++i) {
        const ImVec2& p0 = points[static_cast<size_t>(std::max(0, i - 1))];
        const ImVec2& p1 = points[static_cast<size_t>(i)];
        const ImVec2& p2 = points[static_cast<size_t>(i + 1)];
        const ImVec2& p3 = points[static_cast<size_t>(std::min(static_cast<int>(points.size()) - 1, i + 2))];

        for (int s = 0; s < samplesPerSegment; ++s) {
            const float t = static_cast<float>(s) / static_cast<float>(samplesPerSegment);
            sampled.push_back(CatmullRom(p0, p1, p2, p3, t));
        }
    }

    sampled.push_back(points.back());
    return sampled;
}

std::vector<ImVec2> PathDrawTool::BuildSampledPathSpherical() const {
    std::vector<ImVec2> primary;
    if (points.size() < 2) {
        return primary;
    }

    if (points.size() == 2) {
        primary.push_back(points[0]);
        primary.push_back(points[1]);
        return primary;
    }

    constexpr int samplesPerArc = 28;

    auto appendArcPath = [](std::vector<ImVec2>& into, const std::vector<ImVec2>& arc) {
        if (arc.empty()) {
            return;
        }

        if (into.empty()) {
            into.insert(into.end(), arc.begin(), arc.end());
            return;
        }

        into.insert(into.end(), arc.begin() + 1, arc.end());
    };

    for (int i = 0; i + 2 < static_cast<int>(points.size()); i += 2) {
        appendArcPath(primary, SampleCircularArc(points[static_cast<size_t>(i)],
                                                 points[static_cast<size_t>(i + 1)],
                                                 points[static_cast<size_t>(i + 2)],
                                                 samplesPerArc));
    }

    if (Distance(primary.back(), points.back()) > 0.001f) {
        primary.push_back(points.back());
    }

    if (points.size() % 2 != 0 || points.size() < 4) {
        return primary;
    }

    std::vector<ImVec2> secondary;
    for (int i = 1; i + 2 < static_cast<int>(points.size()); i += 2) {
        appendArcPath(secondary, SampleCircularArc(points[static_cast<size_t>(i)],
                                                   points[static_cast<size_t>(i + 1)],
                                                   points[static_cast<size_t>(i + 2)],
                                                   samplesPerArc));
    }

    if (secondary.empty()) {
        return primary;
    }

    secondary.insert(secondary.begin(), points.front());
    if (Distance(secondary.back(), points.back()) > 0.001f) {
        secondary.push_back(points.back());
    }

    const int targetCount = std::max(static_cast<int>(primary.size()), static_cast<int>(secondary.size()));
    auto a = ResamplePolyline(primary, targetCount);
    auto b = ResamplePolyline(secondary, targetCount);

    std::vector<ImVec2> blended;
    blended.reserve(static_cast<size_t>(targetCount));
    for (int i = 0; i < targetCount; ++i) {
        blended.push_back(Lerp(a[static_cast<size_t>(i)], b[static_cast<size_t>(i)], 0.5f));
    }

    return blended;
}

std::vector<ImVec2> PathDrawTool::BuildSampledPath() const {
    if (smoothingMode == SmoothingMode::Spherical) {
        return BuildSampledPathSpherical();
    }
    return BuildSampledPathStandard();
}

void PathDrawTool::PaintDisc(Model::Layer& layer, const ImVec2& center, float radius, uint32_t packedColor) {
    const int minX = static_cast<int>(std::floor(center.x - radius));
    const int maxX = static_cast<int>(std::ceil(center.x + radius));
    const int minY = static_cast<int>(std::floor(center.y - radius));
    const int maxY = static_cast<int>(std::ceil(center.y + radius));
    const float rr = radius * radius;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (x < 0 || y < 0 || x >= layer.width || y >= layer.height) {
                continue;
            }

            const float dx = static_cast<float>(x) - center.x;
            const float dy = static_cast<float>(y) - center.y;
            if (dx * dx + dy * dy > rr) {
                continue;
            }

            const uint32_t dst = layer.GetPixel(x, y);
            layer.SetPixel(x, y, AlphaBlendOver(dst, packedColor));
        }
    }
}

void PathDrawTool::CommitToLayer(const CanvasContext& ctx) {
    if (ctx.selectedLayer == nullptr || ctx.selectedLayerIndex < 0 || points.size() < 2) {
        return;
    }

    auto sampled = BuildSampledPath();
    if (sampled.size() < 2) {
        return;
    }

    float totalLength = 0.0f;
    for (size_t i = 1; i < sampled.size(); ++i) {
        totalLength += Distance(sampled[i - 1], sampled[i]);
    }
    if (totalLength <= 0.0f) {
        return;
    }

    App& app = App::GetInstance();
    const int layerIndex = ctx.selectedLayerIndex;
    const uint32_t packedColor = PackRgba8(color);

    app.edit_history.Execute(app.project, "Commit Path Stroke", [&](Model::Project& project) {
        if (layerIndex < 0 || layerIndex >= static_cast<int>(project.layers.size())) {
            return;
        }

        Model::Layer& layer = project.layers[static_cast<size_t>(layerIndex)];
        float traveled = 0.0f;

        for (size_t i = 1; i < sampled.size(); ++i) {
            const ImVec2 a = sampled[i - 1];
            const ImVec2 b = sampled[i];
            const float segLength = Distance(a, b);
            if (segLength <= 0.0f) {
                continue;
            }

            const int stamps = std::max(1, static_cast<int>(std::ceil(segLength)));
            for (int s = 0; s <= stamps; ++s) {
                const float lt = static_cast<float>(s) / static_cast<float>(stamps);
                const ImVec2 p = Lerp(a, b, lt);
                const float globalT = std::clamp((traveled + segLength * lt) / totalLength, 0.0f, 1.0f);
                const float r = std::max(0.5f, EvaluateThickness(globalT));
                PaintDisc(layer, p, r, packedColor);
            }

            traveled += segLength;
        }
    });

    points.clear();
    draggingPointIndex = -1;
}

bool PathDrawTool::ProcessCanvasInput(const CanvasContext& ctx) {
    if (ctx.selectedLayer == nullptr) {
        return false;
    }

    const bool inCanvas = (ctx.hovered || ctx.active);
    bool changed = false;

    if (inCanvas && ImGui::IsKeyPressed(ImGuiKey_Backspace, false) && !points.empty()) {
        points.pop_back();
        draggingPointIndex = -1;
        changed = true;
    }

    if (inCanvas && ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
        CommitToLayer(ctx);
        changed = true;
    }

    const int hoveredPoint = FindHoveredPoint(ctx);

    if (inCanvas && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hoveredPoint >= 0) {
        points.erase(points.begin() + hoveredPoint);
        draggingPointIndex = -1;
        changed = true;
    }

    if (!inCanvas || ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            draggingPointIndex = -1;
        }
        return changed;
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (hoveredPoint >= 0) {
            draggingPointIndex = hoveredPoint;
        } else {
            ImVec2 p = MouseToImage(ctx);
            p.x = std::clamp(p.x, 0.0f, static_cast<float>(ctx.project.w - 1));
            p.y = std::clamp(p.y, 0.0f, static_cast<float>(ctx.project.h - 1));
            points.push_back(p);
            draggingPointIndex = static_cast<int>(points.size()) - 1;
            changed = true;
        }
    }

    if (draggingPointIndex >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left) && draggingPointIndex < static_cast<int>(points.size())) {
        ImVec2 p = MouseToImage(ctx);
        p.x = std::clamp(p.x, 0.0f, static_cast<float>(ctx.project.w - 1));
        p.y = std::clamp(p.y, 0.0f, static_cast<float>(ctx.project.h - 1));
        points[static_cast<size_t>(draggingPointIndex)] = p;
        changed = true;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        draggingPointIndex = -1;
    }

    return changed;
}

void PathDrawTool::DrawOverlay(const CanvasContext& ctx, ImDrawList* drawList) {
    if (drawList == nullptr) {
        return;
    }

    const auto toScreen = [&](const ImVec2& p) {
        return ImVec2(ctx.imageOrigin.x + p.x * ctx.zoom, ctx.imageOrigin.y + p.y * ctx.zoom);
    };

    const uint32_t packed = PackRgba8(color);
    const ImU32 pathColor = IM_COL32((packed >> 24) & 0xFFu, (packed >> 16) & 0xFFu, (packed >> 8) & 0xFFu, 230);
    const ImU32 pointColor = IM_COL32(255, 255, 255, 255);
    const ImU32 hoveredColor = IM_COL32(255, 190, 80, 255);

    if (points.size() >= 2) {
        const auto sampled = BuildSampledPath();
        for (size_t i = 1; i < sampled.size(); ++i) {
            drawList->AddLine(toScreen(sampled[i - 1]), toScreen(sampled[i]), pathColor, 2.0f);
        }
    } else if (points.size() == 1) {
        const ImVec2 sp = toScreen(points.front());
        drawList->AddCircleFilled(sp, 3.0f, pointColor);
    }

    const int hoveredPoint = FindHoveredPoint(ctx);

    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        const ImVec2 sp = toScreen(points[static_cast<size_t>(i)]);
        const bool isHovered = (i == hoveredPoint);
        const ImU32 c = isHovered ? hoveredColor : pointColor;
        drawList->AddCircleFilled(sp, 4.0f, c);
        drawList->AddCircle(sp, 6.0f, IM_COL32(0, 0, 0, 220), 0, 1.5f);
    }
}

void PathDrawTool::DrawSettingsUi() {
    ImGui::TextUnformatted("Path Draw Tool Settings");
    ImGui::Text("Points: %d", static_cast<int>(points.size()));

    int smoothingSelection = smoothingMode == SmoothingMode::Standard ? 0 : 1;
    ImGui::RadioButton("Standard", &smoothingSelection, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Spherical", &smoothingSelection, 1);
    smoothingMode = smoothingSelection == 0 ? SmoothingMode::Standard : SmoothingMode::Spherical;

    ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color));

    ImGui::SliderFloat("Thickness Start", &thicknessStart, 0.5f, 64.0f, "%.2f");
    ImGui::SliderFloat("Thickness Middle", &thicknessMiddle, 0.5f, 64.0f, "%.2f");
    ImGui::SliderFloat("Thickness End", &thicknessEnd, 0.5f, 64.0f, "%.2f");
    ImGui::SliderFloat("Thickest Position", &thickestPosition, 0.0f, 1.0f, "%.2f");

    if (ImGui::Button("Clear Path")) {
        points.clear();
        draggingPointIndex = -1;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("LMB click: add point");
    ImGui::TextUnformatted("LMB hold/drag: adjust new point or move existing point");
    ImGui::TextUnformatted("RMB click: remove hovered point");
    ImGui::TextUnformatted("Backspace: remove most recent point");
    ImGui::TextUnformatted("Space: commit path to selected layer");
}

} // namespace app
