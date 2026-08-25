// ClipEditorWindow.hpp
#pragma once

#include "../gui/main/GuiWindow.h"
#include "App.hpp"
#include "NoteSelectInteraction.hpp"
#include "NotePaintInteraction.hpp"
#include "model/Track.hpp"
#include <imgui.h>

namespace gsr::gui {

enum class PianoRollEditMode { Select, Paint };

class ClipEditorWindow : public app::GuiWindow {
public:
    explicit ClipEditorWindow(gsr::App& app);
    ~ClipEditorWindow() = default;

    std::string GetWindowId() override { return "Piano Roll"; }
    void DrawSelf() override;

private:
    gsr::App& m_app;
    
    // Mode Interaction Handlers
    PianoRollEditMode m_edit_mode = PianoRollEditMode::Select;
    NoteSelectInteraction m_select_interaction;
    NotePaintInteraction m_paint_interaction;

    float m_note_height = 14.0f;
    float m_px_per_tick = 0.04f;
    uint32_t m_grid_snap_ticks = 240;

    // Internal Playhead State
    uint64_t m_internal_playhead_tick = 0;
    bool m_was_playing = false;

    Model::Clip* GetSelectedClip();
    static std::string GetPitchName(uint8_t pitch);
    void DrawPianoKeys(ImDrawList* draw_list, ImVec2 origin, float key_width, float total_height);
    void DrawGridBackground(ImDrawList* draw_list, Model::Clip& clip, ImVec2 origin, ImVec2 grid_size);
    void DrawPlayhead(ImDrawList* draw_list, const Model::Clip& clip, ImVec2 grid_origin, ImVec2 grid_size, bool is_focused);
};

} // namespace gsr::gui