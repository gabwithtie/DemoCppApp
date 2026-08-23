#pragma once

#include "model/Track.hpp"

namespace gsr {
class App;
}

namespace gsr::gui {

class NoteManager;

template <typename T>
struct ModalSession {
    bool active = false;
    bool just_started = false;
    T data{};

    // Call each frame. Handles press, active session, and automatic cleanup on release.
    bool Begin(bool is_held) {
        just_started = false;
        if (is_held) {
            if (!active) {
                active = true;
                just_started = true;
                data = T{}; // Reset to clean state on start
            }
        } else if (active) {
            Cancel(); // Auto-cleanup when key is released
        }
        return active;
    }

    // Aborts active session immediately if conditions aren't met
    void Cancel() {
        active = false;
        data = T{};
    }
};

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