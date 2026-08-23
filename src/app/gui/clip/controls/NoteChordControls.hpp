// NoteChordControls.hpp
#pragma once

#include "INoteControls.hpp"
#include <vector>

namespace gsr::gui {

class NoteChordControls : public INoteControls {
public:
    NoteChordControls() = default;

    void HandleKeyboardShortcuts(NoteEditorContext& ctx) override {}
    void DrawContextMenu(NoteEditorContext& ctx) override;

private:
    void BuildChords(NoteEditorContext& ctx, const std::vector<int>& semitone_offsets);
};

} // namespace gsr::gui