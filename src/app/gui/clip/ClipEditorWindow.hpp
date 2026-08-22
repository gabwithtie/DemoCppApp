#pragma once

#include "../gui/main/GuiWindow.h"
#include "App.hpp"
#include "NoteManager.hpp"
#include "model/Track.hpp"
#include <imgui.h>

namespace gsr::gui {

class ClipEditorWindow : public app::GuiWindow {
public:
    explicit ClipEditorWindow(gsr::App& app);
    ~ClipEditorWindow() = default;

    std::string GetWindowId() override { return "Piano Roll"; }
    void DrawSelf() override;

private:
    gsr::App& m_app;
    NoteManager m_note_manager;

    float m_note_height = 14.0f;
    float m_px_per_tick = 0.04f;
    uint32_t m_grid_snap_ticks = 240;

    Model::Clip* GetSelectedClip();
    static std::string GetPitchName(uint8_t pitch);
    void DrawPianoKeys(ImDrawList* draw_list, ImVec2 origin, float key_width, float total_height);
    void DrawGridBackground(ImDrawList* draw_list, Model::Clip& clip, ImVec2 origin, ImVec2 grid_size);
};

} // namespace gsr::gui