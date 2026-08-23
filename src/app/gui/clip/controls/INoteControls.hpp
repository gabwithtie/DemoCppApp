#pragma once

#include "model/Track.hpp"

namespace gsr {
class App;
}

namespace gsr::gui {

class NoteManager;

class INoteControls {
public:
    explicit INoteControls(NoteManager& note_manager)
        : m_note_manager(&note_manager) {}

    virtual ~INoteControls() = default;

    virtual void HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip) = 0;
    
    // Optional context menu items rendered when right-clicking the canvas/notes
    virtual void DrawContextMenu(gsr::App& app, Model::Clip& clip) {}

protected:
    NoteManager* m_note_manager{nullptr};
};

} // namespace gsr::gui