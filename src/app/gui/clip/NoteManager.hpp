#pragma once

#include "App.hpp"
#include "model/Track.hpp"
#include <imgui.h>
#include <vector>

namespace gsr::gui {

struct NoteInitialState {
    size_t index;
    uint64_t start_tick;
    uint64_t duration;
    uint8_t pitch;
};

class NoteManager {
public:
    NoteManager() = default;
    ~NoteManager() = default;

    void ProcessAndDrawNotes(
        gsr::App& app,
        Model::Clip& clip,
        ImVec2 grid_origin,
        ImVec2 grid_size,
        float px_per_tick,
        float note_height,
        uint32_t grid_snap_ticks,
        bool canvas_hovered
    );

private:
    std::vector<Model::Note> m_note_clipboard;

    // Interaction State
    int m_active_note_idx = -1;
    bool m_is_resizing = false;
    bool m_is_dragging = false;

    // Group Drag/Resize Initial States
    std::vector<NoteInitialState> m_selected_initial_states;

    // Box Selection State
    bool m_is_box_selecting = false;
    ImVec2 m_box_select_start{0.0f, 0.0f};

    ImVec2 m_drag_mouse_start{0.0f, 0.0f};

    void HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip);
    void DrawPlayhead(gsr::App& app, const Model::Clip& clip, ImVec2 grid_origin, ImVec2 grid_size, float px_per_tick, ImDrawList* draw_list);
};

} // namespace gsr::gui