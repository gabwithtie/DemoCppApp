// NotePaintInteraction.cpp
#include "NotePaintInteraction.hpp"

namespace gsr::gui {

void NotePaintInteraction::EnsureNoteSegments(Model::Note& note, uint64_t seg_res) {
    size_t count = std::max<size_t>(1, (note.duration + seg_res - 1) / seg_res);
    if (note.paint_segments.size() != count) {
        float default_val = static_cast<float>(note.velocity) / 127.0f;
        note.paint_segments.resize(count, default_val);
    }
}

void NotePaintInteraction::SyncVelocityFromSegments(Model::Note& note) {
    if (!note.paint_segments.empty()) {
        note.velocity = static_cast<uint8_t>(std::clamp(note.paint_segments[0] * 127.0f, 0.0f, 127.0f));
    }
}

void NotePaintInteraction::HandleShortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    // Shift + R + Mouse Scroll = Adjust Brush Radius
    if (m_radius_session.Begin(io.KeyShift && ImGui::IsKeyDown(ImGuiKey_R))) {
        if (io.MouseWheel != 0.0f) {
            brush_radius = std::clamp(brush_radius + io.MouseWheel * 2.0f, 5.0f, 200.0f);
            io.MouseWheel = 0.0f; // Consume mouse wheel event
        }
        return;
    }

    // Shift + S + Mouse Scroll = Adjust Brush Strength (Slower/Narrower scale)
    if (m_strength_session.Begin(io.KeyShift && ImGui::IsKeyDown(ImGuiKey_S))) {
        if (io.MouseWheel != 0.0f) {
            brush_strength = std::clamp(brush_strength + io.MouseWheel * 0.002f, 0.001f, 0.100f);
            io.MouseWheel = 0.0f; // Consume mouse wheel event
        }
        return;
    }
}

void NotePaintInteraction::ProcessPaint(
    gsr::App& app,
    Model::Clip& clip,
    ImVec2 mouse_pos,
    ImVec2 grid_origin,
    float px_per_tick,
    float note_height,
    bool canvas_hovered,
    ImDrawList* draw_list
) {
    // Check shortcuts before hovered canvas execution
    HandleShortcuts();

    if (!canvas_hovered) return;

    // Render brush indicator overlay
    draw_list->AddCircle(mouse_pos, brush_radius, IM_COL32(255, 255, 255, 180), 32, 1.5f);
    draw_list->AddCircleFilled(mouse_pos, brush_radius, IM_COL32(255, 255, 255, 20));

    ImGuiIO& io = ImGui::GetIO();
    bool mouse_moved = (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && mouse_moved) {
        app.SaveUndoPoint();
        ApplyPaintBrush(app, clip, mouse_pos, grid_origin, px_per_tick, note_height);
    }
}

void NotePaintInteraction::ApplyPaintBrush(
    gsr::App& app,
    Model::Clip& clip,
    ImVec2 mouse_pos,
    ImVec2 grid_origin,
    float px_per_tick,
    float note_height
) {
    ImGuiIO& io = ImGui::GetIO();

    bool is_subtract = io.KeyShift;
    float direction = is_subtract ? -1.0f : 1.0f;

    for (auto& note : clip.notes) {
        EnsureNoteSegments(note, SEGMENT_TICK_RES);

        float ny1 = grid_origin.y + (127 - note.pitch) * note_height;
        float ny2 = ny1 + note_height;

        if (mouse_pos.y < ny1 - brush_radius || mouse_pos.y > ny2 + brush_radius) {
            continue;
        }

        bool updated = false;
        size_t seg_count = note.paint_segments.size();

        for (size_t s = 0; s < seg_count; ++s) {
            uint64_t seg_start = note.start_tick + (s * SEGMENT_TICK_RES);
            uint64_t seg_dur = std::min(SEGMENT_TICK_RES, note.duration - (s * SEGMENT_TICK_RES));

            float sx1 = grid_origin.x + (seg_start * px_per_tick);
            float sx2 = sx1 + (seg_dur * px_per_tick);

            ImVec2 seg_center((sx1 + sx2) * 0.5f, (ny1 + ny2) * 0.5f);
            float dx = mouse_pos.x - seg_center.x;
            float dy = mouse_pos.y - seg_center.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= brush_radius) {
                float falloff = 1.0f - (dist / brush_radius);
                float delta = direction * brush_strength * falloff;

                note.paint_segments[s] = std::clamp(
                    note.paint_segments[s] + delta,
                    0.0f, 1.0f
                );
                updated = true;
            }
        }

        if (updated && target == PaintTarget::Velocity) {
            SyncVelocityFromSegments(note);
        }
    }
}

} // namespace gsr::gui