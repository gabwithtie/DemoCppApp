// CtrlControls.hpp
#pragma once

#include "INoteControls.hpp"

namespace gsr::gui {

class CtrlControls : public INoteControls {
public:
    CtrlControls() = default;

    void HandleKeyboardShortcuts(NoteEditorContext& ctx) override;
    void DrawContextMenu(NoteEditorContext& ctx) override;
};

} // namespace gsr::gui