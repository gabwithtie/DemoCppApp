// NotePaintInteraction.hpp
#pragma once

#include "App.hpp"
#include "model/Track.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace gsr::gui {

enum class PaintTarget { Velocity, Aftertouch };

class NotePaintInteraction {
public:
    PaintTarget target = PaintTarget::Velocity;
    float brush_radius = 25.0f;
    float brush_strength = 0.8f;

    static constexpr uint64_t SEGMENT_TICK_RES = 24; // 1/64 note at 960 PPQ

    void ProcessPaint(
        gsr::App& app,
        Model::Clip& clip,
        ImVec2 mouse_pos,
        ImVec2 grid_origin,
        float px_per_tick,
        float note_height,
        bool canvas_hovered,
        ImDrawList* draw_list
    );

    static void EnsureNoteSegments(Model::Note& note, uint64_t seg_res);
    static void SyncVelocityFromSegments(Model::Note& note);

private:
    void ApplyPaintBrush(
        gsr::App& app,
        Model::Clip& clip,
        ImVec2 mouse_pos,
        ImVec2 grid_origin,
        float px_per_tick,
        float note_height
    );
};

} // namespace gsr::gui