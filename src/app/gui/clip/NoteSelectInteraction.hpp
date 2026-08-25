#pragma once

#include "App.hpp"
#include "model/Track.hpp"
#include <imgui.h>
#include <vector>

#include "controls/INoteControls.hpp"
#include "controls/CtrlControls.hpp"
#include "controls/NoteEditingControls.hpp"
#include "controls/NoteSelection.hpp"
#include "controls/NoteChordControls.hpp"
#include "controls/NoteSubdivideControls.hpp"

namespace gsr::gui {

struct NoteInitialState {
    size_t index;
    uint64_t start_tick;
    uint64_t duration;
    uint8_t pitch;
};

class NoteSelectInteraction {
public:
    NoteSelectInteraction();
    ~NoteSelectInteraction() = default;

    void ProcessSelect(
        gsr::App& app,
        Model::Clip& clip,
        uint64_t& internal_playhead_tick,
        ImVec2 mouse_pos,
        ImVec2 grid_origin,
        float& px_per_tick,
        float note_height,
        uint32_t& grid_snap_ticks, // Changed to reference
        bool canvas_hovered,
        int hovered_note_idx,
        bool edge_hovered
    );

    std::vector<Model::Note>& GetNoteClipboard() { return m_note_clipboard; }

private:
    std::vector<Model::Note> m_note_clipboard;

    // Sub-controls
    CtrlControls m_controls_CtrlControls;
    NoteEditingControls m_controls_NoteEditingControls;
    NoteSelection m_controls_NoteSelection;
    NoteChordControls m_controls_NoteChordControls;
    NoteSubdivideControls m_controls_NoteSubdivideControls;
    std::vector<INoteControls*> m_controls;

    // Selection & Drag state
    int m_active_note_idx = -1;
    bool m_is_resizing = false;
    bool m_is_dragging = false;

    std::vector<NoteInitialState> m_selected_initial_states;
    bool m_is_box_selecting = false;
    ImVec2 m_box_select_start{0.0f, 0.0f};
    ImVec2 m_drag_mouse_start{0.0f, 0.0f};

    void HandleKeyboardShortcuts(NoteEditorContext& ctx);
};

} // namespace gsr::gui