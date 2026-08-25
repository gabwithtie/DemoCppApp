// NoteSubdivideControls.hpp
#pragma once

#include "INoteControls.hpp"

namespace gsr::gui {

enum class UnsubdivideMode {
    Once,
    Full
};

class NoteSubdivideControls : public INoteControls {
public:
    NoteSubdivideControls() = default;

    void HandleKeyboardShortcuts(NoteEditorContext& ctx) override {}
    void DrawContextMenu(NoteEditorContext& ctx) override;

private:
    void SubdivideNotes(NoteEditorContext& ctx, int divisions);
    void UnsubdivideNotes(NoteEditorContext& ctx, UnsubdivideMode mode);
};

} // namespace gsr::gui