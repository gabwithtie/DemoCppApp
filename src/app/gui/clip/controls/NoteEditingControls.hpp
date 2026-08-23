// NoteEditingControls.hpp
#pragma once

#include "INoteControls.hpp"

namespace gsr::gui {

class NoteEditingControls : public INoteControls {
public:
    NoteEditingControls() = default;

    void HandleKeyboardShortcuts(NoteEditorContext& ctx) override;
};

} // namespace gsr::gui