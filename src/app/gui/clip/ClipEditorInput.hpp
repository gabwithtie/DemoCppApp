#pragma once

#include "App.hpp"
#include "NotePaintInteraction.hpp"
#include "NoteSelectInteraction.hpp"
#include "model/Track.hpp"
#include <imgui.h>

namespace gsr::gui
{
    enum class PianoRollEditMode { Select, Paint };

    class ClipEditorInput
    {
    public:
        ClipEditorInput() = default;
        ~ClipEditorInput() = default;

        // Process vertical scroll and zoom interactions
        void ProcessVerticalScrollAndZoom(
            bool scroll_area_hovered,
            float outer_height,
            float outer_scroll_y,
            float &note_height,
            float &pending_vertical_scroll);

        // Process horizontal scroll and zoom interactions
        void ProcessHorizontalScrollAndZoom(
            bool scroll_area_hovered,
            float grid_viewport_w,
            float &px_per_tick,
            float &pending_horizontal_scroll);

        // Position internal playhead via Alt + Click/Drag
        void ProcessPlayheadPositioning(
            bool canvas_hovered,
            ImVec2 mouse_pos,
            ImVec2 grid_origin,
            float px_per_tick,
            uint32_t grid_snap_ticks,
            uint64_t clip_duration,
            uint64_t &internal_playhead_tick);

        // Process note paint/select interactions on canvas
        void ProcessNoteInteractions(
            gsr::App &app,
            Model::Clip &clip,
            PianoRollEditMode edit_mode,
            NotePaintInteraction &paint_interaction,
            NoteSelectInteraction &select_interaction,
            uint64_t internal_playhead_tick,
            ImVec2 mouse_pos,
            ImVec2 grid_origin,
            float px_per_tick,
            float note_height,
            uint32_t grid_snap_ticks,
            bool canvas_hovered,
            int hovered_note_idx,
            bool edge_hovered,
            ImDrawList *draw_list);

    private:
        ImGuiKey IsOnlyShiftDown();
    };

} // namespace gsr::gui