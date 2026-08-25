#include "ClipEditorInput.hpp"
#include <algorithm>
#include <cmath>

namespace gsr::gui
{
    ImGuiKey ClipEditorInput::IsOnlyShiftDown()
    {
        ImGuiIO &io = ImGui::GetIO();

        if (io.KeyMods != ImGuiMod_Shift)
            return ImGuiKey::ImGuiKey_None;

        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k)
        {
            ImGuiKey key = static_cast<ImGuiKey>(k);

            if (key >= ImGuiKey_LeftCtrl && key <= ImGuiKey_RightSuper) continue;
            if (key >= ImGuiKey_ReservedForModCtrl && key <= ImGuiKey_ReservedForModSuper) continue;
            if (key >= ImGuiKey_MouseLeft && key <= ImGuiKey_GamepadRStickRight) continue;

            if (ImGui::IsKeyDown(key))
                return key;
        }

        return ImGuiKey::ImGuiKey_None;
    }

    void ClipEditorInput::ProcessVerticalScrollAndZoom(
        bool scroll_area_hovered,
        float outer_height,
        float outer_scroll_y,
        float &note_height,
        float &pending_vertical_scroll)
    {
        ImGuiIO &io = ImGui::GetIO();

        if (scroll_area_hovered || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            if (io.KeyAlt && io.MouseWheel != 0.0f)
            {
                const float old_note_height = note_height;
                const float half_vh = outer_height * 0.5f;
                const float center_pitch_unit = (outer_scroll_y + half_vh) / old_note_height;

                float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
                note_height = std::clamp(note_height * zoom_factor, 8.0f, 28.0f);

                pending_vertical_scroll = std::max(0.0f, center_pitch_unit * note_height - half_vh);
            }
            if (!io.KeyShift && !io.KeyAlt && io.MouseWheel != 0.0f)
            {
                pending_vertical_scroll = std::max(0.0f, outer_scroll_y - io.MouseWheel * ImGui::GetFontSize() * 3.0f);
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.MouseDelta.y != 0.0f)
            {
                const float base_y = (pending_vertical_scroll >= 0.0f) ? pending_vertical_scroll : outer_scroll_y;
                pending_vertical_scroll = std::max(0.0f, base_y - io.MouseDelta.y);
            }
        }
    }

    void ClipEditorInput::ProcessHorizontalScrollAndZoom(
        bool scroll_area_hovered,
        float grid_viewport_w,
        float &px_per_tick,
        float &pending_horizontal_scroll)
    {
        ImGuiIO &io = ImGui::GetIO();

        if (scroll_area_hovered || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            auto other_key = IsOnlyShiftDown();
            auto other_key_is_scroll = (other_key == ImGuiKey::ImGuiKey_MouseWheelY);

            if (other_key_is_scroll && io.MouseWheel != 0.0f)
            {
                const float old_px_per_tick = px_per_tick;
                const float half_vw = grid_viewport_w * 0.5f;
                const float current_scroll_x = ImGui::GetScrollX();
                const float center_tick = (current_scroll_x + half_vw) / old_px_per_tick;

                float zoom_factor = (io.MouseWheel > 0.0f) ? 1.05f : 0.95f;
                px_per_tick = std::clamp(px_per_tick * zoom_factor, 0.005f, 0.2f);

                pending_horizontal_scroll = std::max(0.0f, center_tick * px_per_tick - half_vw);
                io.MouseWheel = 0.0f;
                io.MouseWheelH = 0.0f;
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.MouseDelta.x != 0.0f)
            {
                const float base_x = (pending_horizontal_scroll >= 0.0f) ? pending_horizontal_scroll : ImGui::GetScrollX();
                pending_horizontal_scroll = std::max(0.0f, base_x - io.MouseDelta.x);
            }
        }
    }

    void ClipEditorInput::ProcessPlayheadPositioning(
        bool canvas_hovered,
        ImVec2 mouse_pos,
        ImVec2 grid_origin,
        float px_per_tick,
        uint32_t grid_snap_ticks,
        uint64_t clip_duration,
        uint64_t &internal_playhead_tick)
    {
        ImGuiIO &io = ImGui::GetIO();

        if (canvas_hovered && io.KeyAlt && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            float rel_x = mouse_pos.x - grid_origin.x;
            if (rel_x >= 0.0f)
            {
                uint64_t target_tick = static_cast<uint64_t>(rel_x / px_per_tick);
                if (grid_snap_ticks > 0)
                {
                    target_tick = (target_tick / grid_snap_ticks) * grid_snap_ticks;
                }
                internal_playhead_tick = std::min(target_tick, clip_duration);
            }
        }
    }

    void ClipEditorInput::ProcessNoteInteractions(
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
        ImDrawList *draw_list)
    {
        if (edit_mode == PianoRollEditMode::Paint)
        {
            paint_interaction.ProcessPaint(
                app, clip, mouse_pos, grid_origin, px_per_tick, note_height, canvas_hovered, draw_list);
        }
        else
        {
            select_interaction.ProcessSelect(
                app, clip, internal_playhead_tick, mouse_pos, grid_origin, px_per_tick, note_height, grid_snap_ticks, canvas_hovered, hovered_note_idx, edge_hovered);
        }
    }

} // namespace gsr::gui