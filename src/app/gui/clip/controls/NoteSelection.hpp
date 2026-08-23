// NoteSelection.hpp
#pragma once

#include "INoteControls.hpp"

namespace gsr::gui {

class NoteSelection : public INoteControls {
public:
    NoteSelection() = default;

    void HandleKeyboardShortcuts(NoteEditorContext& ctx) override;
};

} // namespace gsr::gui