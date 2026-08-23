#pragma once

#include "model/Track.hpp"
#include <vector>
#include <cstdint>

namespace gsr {
class App;
}

namespace gsr::gui {

// Lightweight context object aggregating frame dependencies
struct NoteEditorContext {
    gsr::App& app;
    Model::Clip& clip;
    std::vector<Model::Note>& clipboard;
    uint32_t grid_snap_ticks;
};

template <typename T>
struct ModalSession {
    bool active = false;
    bool just_started = false;
    T data{};

    bool Begin(bool is_held) {
        just_started = false;
        if (is_held) {
            if (!active) {
                active = true;
                just_started = true;
                data = T{};
            }
        } else if (active) {
            Cancel();
        }
        return active;
    }

    void Cancel() {
        active = false;
        data = T{};
    }
};

class INoteControls {
public:
    virtual ~INoteControls() = default;

    // Process keyboard shortcuts for this control module
    virtual void HandleKeyboardShortcuts(NoteEditorContext& ctx) = 0;
    
    // Optional context menu items rendered on right-click
    virtual void DrawContextMenu(NoteEditorContext& ctx) {}
};

} // namespace gsr::gui