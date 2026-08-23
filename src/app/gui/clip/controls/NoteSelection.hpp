#pragma once

#include "INoteControls.hpp"

namespace gsr::gui {

class NoteSelection : public INoteControls {
public:
    using INoteControls::INoteControls;

    void HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip) override;
};

} // namespace gsr::gui