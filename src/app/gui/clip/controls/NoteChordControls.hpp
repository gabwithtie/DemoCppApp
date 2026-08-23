#pragma once

#include "INoteControls.hpp"
#include <vector>

namespace gsr::gui {

class NoteChordControls : public INoteControls {
public:
    using INoteControls::INoteControls;

    void HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip) override {}
    void DrawContextMenu(gsr::App& app, Model::Clip& clip) override;

private:
    void BuildChords(gsr::App& app, Model::Clip& clip, const std::vector<int>& semitone_offsets);
};

} // namespace gsr::gui